#pragma once

#include <memory>
#include <vector>

#include <QPluginLoader>
#include <QString>
#include <QStringList>

namespace cappy::plugin_api {
class IPlugin;
}

namespace cappy::plugin_host {

class PluginManager {
  public:
    ~PluginManager();

    bool loadFromDirectory(const QString& path);
    QStringList loadedPluginIds() const;

  private:
    std::vector<std::unique_ptr<QPluginLoader>> loaders_;
    std::vector<cappy::plugin_api::IPlugin*> plugins_;
};

} // namespace cappy::plugin_host
