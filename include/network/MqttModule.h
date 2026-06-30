#pragma once

#include "core/IModule.h"
#include "core/DeviceConstants.h"
#include "core/EventBus.h"
#include "config/ConfigModule.h"
#include "network/NetworkModule.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

class MqttModule : public IModule {
public:
    MqttModule(ConfigModule& config, NetworkModule& network, EventBus& eventBus);

    void init() override;
    void loop() override;

    bool isConnected();
    int getLastConnectState() const;
    void reloadSettings();

private:
    ConfigModule& config_;
    NetworkModule& network_;
    EventBus& eventBus_;
    WiFiClient wifiClient_;
    PubSubClient client_;

    String brokerHost_;
    String mqttUser_;
    String mqttPass_;
    String topicPrefix_;
    String topicDeviceId_;
    int lastConnectState_ = 0;

    bool networkUp_ = false;
    bool started_ = false;
    unsigned long lastReconnectAttempt_ = 0;
    unsigned long lastStatusPublish_ = 0;
    unsigned long reconnectDelayMs_ = 0;

    char topicDisplaySet_[96];
    char topicDisplayRestore_[96];
    char topicStatus_[96];
    char topicAvailability_[96];

    void onNetworkConnected(const Event& event);
    void onNetworkDisconnected(const Event& event);
    void start();
    void stop();
    void rebuildTopics();
    bool connectBroker();
    void subscribeTopics();
    void publishStatus(bool online);
    void publishDiscovery();
    void handleMessage(const char* topic, const byte* payload, unsigned int length);

    static void callbackThunk(char* topic, byte* payload, unsigned int length);
    static MqttModule* instance_;
};
