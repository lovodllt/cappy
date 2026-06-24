#pragma once

#include <memory>

#include <QLockFile>
#include <QStringList>

#include "AppDirectories.h"
#include "AppSettings.h"
#include "cappy/plugin_host/PluginManager.h"

class QApplication;
class AppController;
class MainWindow;

class AppBootstrap {
  public:
    explicit AppBootstrap(QApplication& app);
    ~AppBootstrap();

    int run();

  private:
    QStringList loadPlugins();

    QApplication& app_;
    AppDirectories directories_;
    std::unique_ptr<AppSettings> settings_;
    std::unique_ptr<QLockFile> singleInstanceLock_;
    cappy::plugin_host::PluginManager pluginManager_;
    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<AppController> controller_;
};
