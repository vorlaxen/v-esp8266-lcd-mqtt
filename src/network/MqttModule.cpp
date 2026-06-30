#include "network/MqttModule.h"
#include "mqtt/MqttMessageParser.h"
#include "mqtt/MqttTopics.h"
#include "core/DeviceConstants.h"
#include "core/Log.h"
#include <ArduinoJson.h>

MqttModule* MqttModule::instance_ = nullptr;

MqttModule::MqttModule(ConfigModule& config, NetworkModule& network, EventBus& eventBus)
    : config_(config),
      network_(network),
      eventBus_(eventBus),
      client_(wifiClient_) {
    instance_ = this;
    client_.setCallback(callbackThunk);
    client_.setBufferSize(Device::kMqttBufferSize);
    client_.setSocketTimeout(15);
    client_.setKeepAlive(60);
}

void MqttModule::init() {
    eventBus_.subscribe(EventType::NetworkConnected,
                        [this](const Event& event) { onNetworkConnected(event); });
    eventBus_.subscribe(EventType::NetworkDisconnected,
                        [this](const Event& event) { onNetworkDisconnected(event); });
}

void MqttModule::loop() {
    if (!started_ || !networkUp_) {
        return;
    }

    if (!client_.connected()) {
        if (millis() - lastReconnectAttempt_ >= reconnectDelayMs_) {
            lastReconnectAttempt_ = millis();
            if (connectBroker()) {
                reconnectDelayMs_ = Device::kMqttReconnectMinMs;
            } else if (reconnectDelayMs_ < Device::kMqttReconnectMaxMs) {
                reconnectDelayMs_ = min(reconnectDelayMs_ * 2, Device::kMqttReconnectMaxMs);
            }
        }
        return;
    }

    client_.loop();

    if (millis() - lastStatusPublish_ >= Device::kMqttStatusIntervalMs) {
        publishStatus(true);
        lastStatusPublish_ = millis();
    }
}

bool MqttModule::isConnected() {
    return started_ && client_.connected();
}

int MqttModule::getLastConnectState() const {
    return lastConnectState_;
}

void MqttModule::reloadSettings() {
    stop();
    if (networkUp_) {
        start();
        lastReconnectAttempt_ = 0;
    }
}

void MqttModule::onNetworkConnected(const Event& event) {
    (void)event;
    networkUp_ = true;
    start();
}

void MqttModule::onNetworkDisconnected(const Event& event) {
    (void)event;
    networkUp_ = false;
    stop();
}

void MqttModule::start() {
    if (!config_.isMqttEnabled() || !config_.hasMqttBroker()) {
        return;
    }

    brokerHost_ = config_.getMqttBroker();
    mqttUser_ = config_.getMqttUsername();
    mqttPass_ = config_.getMqttPassword();

    rebuildTopics();
    client_.setServer(brokerHost_.c_str(), config_.getMqttPort());
    started_ = true;
    reconnectDelayMs_ = Device::kMqttReconnectMinMs;
    lastReconnectAttempt_ = 0;
    lastStatusPublish_ = 0;
    Log::info("MQTT module started");
}

void MqttModule::stop() {
    if (client_.connected()) {
        publishStatus(false);
        client_.disconnect();
    }
    started_ = false;
}

void MqttModule::rebuildTopics() {
    topicPrefix_ = config_.getMqttTopicPrefix();
    topicDeviceId_ = config_.getDeviceId();

    const MqttTopicContext ctx = {topicPrefix_.c_str(), topicDeviceId_.c_str()};

    mqttTopicDisplaySet(topicDisplaySet_, sizeof(topicDisplaySet_), &ctx);
    mqttTopicDisplayRestore(topicDisplayRestore_, sizeof(topicDisplayRestore_), &ctx);
    mqttTopicStatus(topicStatus_, sizeof(topicStatus_), &ctx);
    mqttTopicAvailability(topicAvailability_, sizeof(topicAvailability_), &ctx);

    Log::infof("MQTT topic: %s", topicDisplaySet_);
}

bool MqttModule::connectBroker() {
    if (!network_.isConnected()) {
        return false;
    }

    const String clientId = String("vorlaxen-") + config_.getDeviceId();
    const char* user = mqttUser_.c_str();
    const char* pass = mqttPass_.c_str();
    const bool hasAuth = user[0] != '\0';

    const char* willTopic = topicAvailability_;
    const char* willMessage = "offline";
    const bool willRetain = true;

    Log::info("MQTT connecting...");

    const bool ok = hasAuth
                        ? client_.connect(clientId.c_str(), user, pass, willTopic, 1, willRetain,
                                          willMessage)
                        : client_.connect(clientId.c_str(), willTopic, 1, willRetain, willMessage);

    lastConnectState_ = client_.state();

    if (!ok) {
        Log::warn("MQTT connect failed");
        return false;
    }

    Log::info("MQTT connected");
    subscribeTopics();
    client_.publish(topicAvailability_, "online", true);
    publishDiscovery();
    publishStatus(true);
    lastStatusPublish_ = millis();
    return true;
}

void MqttModule::subscribeTopics() {
    client_.subscribe(topicDisplaySet_);
    client_.subscribe(topicDisplayRestore_);
}

void MqttModule::publishStatus(bool online) {
    if (!client_.connected()) {
        return;
    }

    JsonDocument doc;
    doc["online"] = online;
    doc["deviceId"] = config_.getDeviceId();
    if (online) {
        doc["ip"] = network_.getIp();
        doc["hostname"] = network_.getHostname();
        doc["ssid"] = network_.getSsid();
        doc["rssi"] = network_.getRssi();
    }

    String payload;
    serializeJson(doc, payload);
    client_.publish(topicStatus_, payload.c_str(), true);
}

void MqttModule::publishDiscovery() {
    if (!client_.connected()) {
        return;
    }

    const String discoveryTopic = String("homeassistant/text/vorlaxen_") +
                                  config_.getDeviceId() + "/config";

    JsonDocument doc;
    doc["name"] = String("Vorlaxen LCD ") + config_.getDeviceId();
    doc["unique_id"] = String("vorlaxen_lcd_") + config_.getDeviceId();
    doc["command_topic"] = topicDisplaySet_;
    doc["state_topic"] = topicStatus_;
    doc["value_template"] = "{{ value_json.text | default('') }}";
    doc["icon"] = "mdi:television";
    doc["max"] = Device::kMaxMessageLen;

    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = String("vorlaxen_") + config_.getDeviceId();
    device["manufacturer"] = "Vorlaxen Labs";
    device["model"] = "ESP8266 LCD";
    device["name"] = String("Vorlaxen ") + config_.getDeviceId();

    JsonObject availability = doc["availability"].to<JsonObject>();
    availability["topic"] = topicAvailability_;
    availability["payload_available"] = "online";
    availability["payload_not_available"] = "offline";

    String payload;
    serializeJson(doc, payload);
    client_.publish(discoveryTopic.c_str(), payload.c_str(), true);
}

void MqttModule::handleMessage(const char* topic, const byte* payload, unsigned int length) {
    if (!topic) {
        return;
    }

    if (strcmp(topic, topicDisplayRestore_) == 0) {
        eventBus_.publish(makeDisplayRestoreNetworkEvent());
        return;
    }

    if (strcmp(topic, topicDisplaySet_) != 0) {
        return;
    }

    const MqttParsedMessage parsed =
        mqttParsePayload(reinterpret_cast<const char*>(payload), length);

    if (parsed.action == MQTT_ACTION_RESTORE) {
        eventBus_.publish(makeDisplayRestoreNetworkEvent());
        return;
    }

    if (parsed.action != MQTT_ACTION_DISPLAY || parsed.text[0] == '\0') {
        Log::warn("MQTT: ignored display payload");
        return;
    }

    Log::infof("MQTT LCD: %s", parsed.text);
    eventBus_.publish(makeDisplayMessageEvent(String(parsed.text), parsed.scroll, parsed.scrollMs));
}

void MqttModule::callbackThunk(char* topic, byte* payload, unsigned int length) {
    if (instance_) {
        instance_->handleMessage(topic, payload, length);
    }
}
