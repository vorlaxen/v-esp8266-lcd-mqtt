#pragma once

#include "config/ConfigModule.h"
#include "config/FactoryResetModule.h"
#include "core/EventBus.h"
#include "core/IModule.h"
#include "display/DisplayModule.h"
#include "network/MqttModule.h"
#include "network/NetworkModule.h"
#include "network/WebServerModule.h"

class Application {
public:
    Application();

    void setup();
    void loop();

private:
    static constexpr size_t kModuleCount = 5;

    EventBus eventBus_;
    ConfigModule config_;
    DisplayModule display_;
    NetworkModule network_;
    MqttModule mqtt_;
    WebServerModule webServer_;
    FactoryResetModule factoryReset_;

    IModule* modules_[kModuleCount];

    void initModules();
};
