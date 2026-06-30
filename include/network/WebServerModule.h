#pragma once

#include "core/IModule.h"
#include "core/DeviceConstants.h"
#include "core/EventBus.h"
#include "config/ConfigModule.h"
#include "network/MqttModule.h"
#include "network/NetworkModule.h"
#include "network/StaticFileHandler.h"
#include <ESP8266WebServer.h>

class WebServerModule : public IModule {
public:
    WebServerModule(ConfigModule& config,
                    NetworkModule& network,
                    MqttModule& mqtt,
                    EventBus& eventBus);

    void init() override;
    void loop() override;

private:
    ConfigModule& config_;
    NetworkModule& network_;
    MqttModule& mqtt_;
    EventBus& eventBus_;
    ESP8266WebServer server_{Device::kHttpPort};
    StaticFileHandler files_;
    bool started_ = false;

    void start();
    void setupRoutes();
    void handleRoot();
    void handleStatus();
    void handleMessage();
    void handleDisplayRestore();
    void handleMqttGet();
    void handleMqttPost();
    void onNetworkConnected(const Event& event);
    void onNetworkDisconnected(const Event& event);
};
