#pragma once

#include <QString>

class AppLogger {
  public:
    static QString initialize(const QString& logsDirectory);
};
