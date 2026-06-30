#pragma once

#include "config/ConfigModule.h"
#include "core/DeviceConstants.h"
#include "core/EventBus.h"
#include "network/StaticFileHandler.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>

class WiFiProvisioner {
public:
    WiFiProvisioner(ConfigModule& config, EventBus& eventBus);

    void start();
    void loop();
    bool isComplete() const;
    bool isActive() const;

private:
    ConfigModule& config_;
    EventBus& eventBus_;
    DNSServer dnsServer_;
    ESP8266WebServer server_{Device::kHttpPort};
    StaticFileHandler files_;
    bool active_ = false;
    bool complete_ = false;

    void setupRoutes();
    bool captivePortal();
    void handleRoot();
    void handleScan();
    void handleConnect();
    void handleCaptiveProbe();
};
