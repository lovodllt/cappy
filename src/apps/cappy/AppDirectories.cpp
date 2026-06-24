#include "AppDirectories.h"

#include <QDir>
#include <QStandardPaths>

AppDirectories AppDirectories::ensureCreated() {
    AppDirectories directories;
    directories.appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    directories.logsDir = directories.appDataDir + "/logs";
    directories.capturesDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        + "/Cappy";
    directories.pluginsDir = directories.appDataDir + "/plugins";

    QDir().mkpath(directories.appDataDir);
    QDir().mkpath(directories.logsDir);
    QDir().mkpath(directories.capturesDir);
    QDir().mkpath(directories.pluginsDir);

    return directories;
}

