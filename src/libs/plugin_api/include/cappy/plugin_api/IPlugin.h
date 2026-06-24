#pragma once

#include <QString>
#include <QtPlugin>

namespace cappy::plugin_api {

struct PluginDescriptor {
    QString id;
    QString name;
    QString version;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual PluginDescriptor descriptor() const = 0;
    virtual void initialize() = 0;
};

}  // namespace cappy::plugin_api

#define CAPPY_IPLUGIN_IID "tech.cappy.plugin.IPlugin/1.0"
Q_DECLARE_INTERFACE(cappy::plugin_api::IPlugin, CAPPY_IPLUGIN_IID)
