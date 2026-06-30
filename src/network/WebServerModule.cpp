#include "network/WebServerModule.h"
#include "network/JsonResponse.h"
#include "core/DeviceConstants.h"
#include <ArduinoJson.h>

WebServerModule::WebServerModule(ConfigModule& config,
                                   NetworkModule& network,
                                   MqttModule& mqtt,
                                   EventBus& eventBus)
    : config_(config), network_(network), mqtt_(mqtt), eventBus_(eventBus), files_(server_) {}

void WebServerModule::init() {
    eventBus_.subscribe(EventType::NetworkConnected,
                        [this](const Event& event) { onNetworkConnected(event); });
    eventBus_.subscribe(EventType::NetworkDisconnected,
                        [this](const Event& event) { onNetworkDisconnected(event); });
}

void WebServerModule::loop() {
    if (started_) {
        server_.handleClient();
    }
}

void WebServerModule::start() {
    if (started_) {
        return;
    }

    setupRoutes();
    server_.begin();
    started_ = true;
}

void WebServerModule::setupRoutes() {
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/css/style.css", HTTP_GET,
               [this]() { files_.serve("/web/css/style.css", "text/css"); });
    server_.on("/js/constants.js", HTTP_GET,
               [this]() { files_.serve("/web/js/constants.js", "application/javascript"); });
    server_.on("/js/ui.js", HTTP_GET,
               [this]() { files_.serve("/web/js/ui.js", "application/javascript"); });
    server_.on("/js/lcd-glyphs.js", HTTP_GET,
               [this]() { files_.serve("/web/js/lcd-glyphs.js", "application/javascript"); });
    server_.on("/js/api.js", HTTP_GET,
               [this]() { files_.serve("/web/js/api.js", "application/javascript"); });
    server_.on("/js/app.js", HTTP_GET,
               [this]() { files_.serve("/web/js/app.js", "application/javascript"); });
    server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server_.on("/api/message", HTTP_POST, [this]() { handleMessage(); });
    server_.on("/api/display/restore", HTTP_POST, [this]() { handleDisplayRestore(); });
    server_.on("/api/mqtt", HTTP_GET, [this]() { handleMqttGet(); });
    server_.on("/api/mqtt", HTTP_POST, [this]() { handleMqttPost(); });
    server_.onNotFound([this]() { files_.sendNotFound(); });
}

void WebServerModule::handleRoot() {
    if (!files_.serve("/web/index.html", "text/html")) {
        server_.send(500, "text/plain", "index.html not found");
    }
}

void WebServerModule::handleStatus() {
    JsonDocument doc;
    doc["ip"] = network_.getIp();
    doc["hostname"] = network_.getHostname();
    doc["deviceId"] = config_.getDeviceId();
    doc["ssid"] = network_.getSsid();
    doc["rssi"] = network_.getRssi();

    String response;
    serializeJson(doc, response);
    server_.send(200, "application/json", response);
}

void WebServerModule::handleMessage() {
    String text;
    bool scroll = true;
    uint16_t scrollMs = Device::kDefaultScrollDelayMs;

    if (server_.hasArg("plain")) {
        JsonDocument doc;
        if (deserializeJson(doc, server_.arg("plain")) == DeserializationError::Ok) {
            text = doc["text"] | "";
            scroll = doc["scroll"] | true;
            scrollMs = doc["scrollMs"] | Device::kDefaultScrollDelayMs;
        }
    } else if (server_.hasArg("text")) {
        text = server_.arg("text");
        scroll = server_.arg("scroll") != "0";
        scrollMs = server_.hasArg("scrollMs")
                       ? server_.arg("scrollMs").toInt()
                       : Device::kDefaultScrollDelayMs;
    }

    text.trim();
    if (text.length() == 0) {
        JsonResponse::sendError(server_, 400, "text_required");
        return;
    }

    if (text.length() > Device::kMaxMessageLen) {
        text = text.substring(0, Device::kMaxMessageLen);
    }

    eventBus_.publish(makeDisplayMessageEvent(text, scroll, scrollMs));
    JsonResponse::sendOk(server_);
}

void WebServerModule::handleDisplayRestore() {
    eventBus_.publish(makeDisplayRestoreNetworkEvent());
    JsonResponse::sendOk(server_);
}

void WebServerModule::handleMqttGet() {
    const MqttSettings settings = config_.getMqttSettings();

    JsonDocument doc;
    doc["enabled"] = settings.enabled;
    doc["broker"] = settings.broker;
    doc["port"] = settings.port;
    doc["username"] = settings.username;
    doc["hasPassword"] = settings.password.length() > 0;
    doc["topicPrefix"] = settings.topicPrefix;
    doc["connected"] = mqtt_.isConnected();
    doc["lastConnectState"] = mqtt_.getLastConnectState();

    String response;
    serializeJson(doc, response);
    server_.send(200, "application/json", response);
}

void WebServerModule::handleMqttPost() {
    if (!server_.hasArg("plain")) {
        JsonResponse::sendError(server_, 400, "invalid_body");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server_.arg("plain")) != DeserializationError::Ok) {
        JsonResponse::sendError(server_, 400, "invalid_json");
        return;
    }

    MqttSettings settings = config_.getMqttSettings();
    settings.enabled = doc["enabled"] | settings.enabled;
    settings.broker = doc["broker"] | settings.broker;
    settings.port = doc["port"] | settings.port;
    settings.username = doc["username"] | settings.username;
    settings.topicPrefix = doc["topicPrefix"] | settings.topicPrefix;

    if (doc["password"].is<const char*>()) {
        settings.password = doc["password"].as<const char*>();
    }

    settings.broker.trim();
    settings.topicPrefix.trim();

    if (settings.enabled && settings.broker.length() == 0) {
        JsonResponse::sendError(server_, 400, "broker_required");
        return;
    }

    if (settings.topicPrefix.length() == 0) {
        settings.topicPrefix = MQTT_TOPIC_PREFIX;
    }

    config_.saveMqttSettings(settings);
    mqtt_.reloadSettings();
    JsonResponse::sendOk(server_);
}

void WebServerModule::onNetworkConnected(const Event& event) {
    (void)event;
    start();
}

void WebServerModule::onNetworkDisconnected(const Event& event) {
    (void)event;
    server_.stop();
    started_ = false;
}
