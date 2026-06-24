#include "AppBootstrap.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QThread>

#include "AppController.h"
#include "AppLogger.h"
#include "MainWindow.h"
#include "cappy/localization/Localization.h"

AppBootstrap::AppBootstrap(QApplication& app)
    : app_(app)
    , directories_(AppDirectories::ensureCreated())
    , settings_(std::make_unique<AppSettings>(directories_.capturesDir)) {
}

AppBootstrap::~AppBootstrap() = default;

int AppBootstrap::run() {
    for (const QString& argument : app_.arguments()) {
        if (!argument.startsWith("--restart-delay-ms=")) {
            continue;
        }

        bool ok = false;
        const int delayMs = argument.mid(QString("--restart-delay-ms=").size()).toInt(&ok);
        if (ok && delayMs > 0) {
            QThread::msleep(static_cast<unsigned long>(delayMs));
        }
        break;
    }

    singleInstanceLock_ = std::make_unique<QLockFile>(
        QDir(directories_.appDataDir).filePath("cappy.instance.lock")
    );
    singleInstanceLock_->setStaleLockTime(0);
    if (!singleInstanceLock_->tryLock()) {
        const auto shellSettings = settings_->loadShellSettings();
        const auto language = cappy::localization::resolvedAppLanguageFromSettings(
            shellSettings.interfaceLanguage
        );
        const auto& text = cappy::localization::strings(language);
        qWarning() << "Another Cappy instance is already running.";
        QMessageBox::warning(
            nullptr,
            text.appName,
            text.singleInstanceAlreadyRunningMessage
        );
        return 1;
    }

    const QStringList pluginIds = loadPlugins();
    const QString logFilePath = AppLogger::initialize(directories_.logsDir);
    const bool smokeTest = app_.arguments().contains("--smoke-test");
    const bool fullscreenCaptureSmokeTest = app_.arguments().contains("--smoke-fullscreen-capture");
    const bool activeWindowCaptureSmokeTest = app_.arguments().contains("--smoke-active-window-capture");
    const bool pinLatestCaptureSmokeTest = app_.arguments().contains("--smoke-pin-latest-capture");

    mainWindow_ = std::make_unique<MainWindow>(pluginIds);
    controller_ = std::make_unique<AppController>(
        app_,
        *mainWindow_,
        *settings_,
        logFilePath
    );
    controller_->initialize();
    QObject::connect(
        &app_,
        &QCoreApplication::aboutToQuit,
        controller_.get(),
        &AppController::persistShellState
    );

    if (smokeTest) {
        qInfo() << "Smoke test completed.";
        controller_->persistShellState();
        return 0;
    }

    if (fullscreenCaptureSmokeTest) {
        qInfo() << "Running fullscreen capture smoke test.";
        controller_->runFullscreenCaptureSmokeTest();
    }

    if (activeWindowCaptureSmokeTest) {
        qInfo() << "Running active-window capture smoke test.";
        controller_->runActiveWindowCaptureSmokeTest();
    }

    if (pinLatestCaptureSmokeTest) {
        qInfo() << "Running pin-latest-capture smoke test.";
        controller_->runPinLatestCaptureSmokeTest();
    }

    return app_.exec();
}

QStringList AppBootstrap::loadPlugins() {
    const QDir appDir(QApplication::applicationDirPath());

    const QString localPluginPath = appDir.filePath("plugins");
    const QString installedPluginPath = appDir.filePath("../lib/cappy/plugins");

    if (!pluginManager_.loadFromDirectory(localPluginPath)) {
        pluginManager_.loadFromDirectory(installedPluginPath);
    }

    return pluginManager_.loadedPluginIds();
}
