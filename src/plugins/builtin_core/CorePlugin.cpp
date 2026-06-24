#include "CorePlugin.h"

cappy::plugin_api::PluginDescriptor CorePlugin::descriptor() const {
    cappy::plugin_api::PluginDescriptor descriptor;
    descriptor.id = "cappy.core";
    descriptor.name = "Core Plugin";
    descriptor.version = "0.1.0";
    return descriptor;
}

void CorePlugin::initialize() {
}
