#pragma once

#include <QString>

struct AppDirectories {
    QString appDataDir;
    QString logsDir;
    QString capturesDir;
    QString pluginsDir;

    static AppDirectories ensureCreated();
};

