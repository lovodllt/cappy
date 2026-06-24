#include "AppLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

namespace {

QMutex g_logMutex;
QFile* g_logFile = nullptr;

QString levelName(QtMsgType type) {
    switch (type) {
    case QtDebugMsg:
        return "DEBUG";
    case QtInfoMsg:
        return "INFO";
    case QtWarningMsg:
        return "WARN";
    case QtCriticalMsg:
        return "ERROR";
    case QtFatalMsg:
        return "FATAL";
    }

    return "UNKNOWN";
}

void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& message) {
    QMutexLocker locker(&g_logMutex);
    if (g_logFile == nullptr || !g_logFile->isOpen()) {
        return;
    }

    QTextStream stream(g_logFile);
    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << " [" << levelName(type)
           << "] " << message << '\n';
    stream.flush();

    if (type == QtFatalMsg) {
        abort();
    }
}

} // namespace

QString AppLogger::initialize(const QString& logsDirectory) {
    QDir().mkpath(logsDirectory);

    const QString logFilePath = logsDirectory + "/cappy.log";

    static QFile file(logFilePath);
    if (!file.isOpen()) {
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

    g_logFile = &file;
    qInstallMessageHandler(messageHandler);
    return logFilePath;
}
