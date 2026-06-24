#pragma once

#include <QObject>

#include "cappy/plugin_api/IPlugin.h"

class CorePlugin final : public QObject, public cappy::plugin_api::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID CAPPY_IPLUGIN_IID FILE "CorePlugin.json")
    Q_INTERFACES(cappy::plugin_api::IPlugin)

public:
    cappy::plugin_api::PluginDescriptor descriptor() const override;
    void initialize() override;
};

