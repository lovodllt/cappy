#include "cappy/plugin_host/PluginManager.h"

#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>

#include "cappy/plugin_api/IPlugin.h"

namespace cappy::plugin_host {

PluginManager::~PluginManager() = default;

bool PluginManager::loadFromDirectory(const QString& path) {
    QDir pluginDir(path);
    if (!pluginDir.exists()) {
        return false;
    }

    const QFileInfoList entries = pluginDir.entryInfoList(QDir::Files);
    bool loadedAny = false;

    for (const QFileInfo& entry : entries) {
        auto loader = std::make_unique<QPluginLoader>(entry.absoluteFilePath());
        QObject* instance = loader->instance();
        if (instance == nullptr) {
            continue;
        }

        auto* plugin = qobject_cast<cappy::plugin_api::IPlugin*>(instance);
        if (plugin == nullptr) {
            loader->unload();
            continue;
        }

        plugin->initialize();
        plugins_.push_back(plugin);
        loaders_.push_back(std::move(loader));
        loadedAny = true;
    }

    return loadedAny;
}

QStringList PluginManager::loadedPluginIds() const {
    QStringList ids;
    ids.reserve(static_cast<qsizetype>(plugins_.size()));

    for (const auto* plugin : plugins_) {
        ids.push_back(plugin->descriptor().id);
    }

    return ids;
}

}  // namespace cappy::plugin_host
