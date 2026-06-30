#include "network/WiFiProvisioner.h"
#include "network/JsonResponse.h"
#include "core/UiStrings.h"
#include "core/DeviceConstants.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

WiFiProvisioner::WiFiProvisioner(ConfigModule& config, EventBus& eventBus)
    : config_(config), eventBus_(eventBus), files_(server_) {}

void WiFiProvisioner::start() {
    if (active_) {
        return;
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(
        IPAddress(Device::kApIp, Device::kApIp2, Device::kApIp3, Device::kApIp4),
        IPAddress(Device::kApIp, Device::kApIp2, Device::kApIp3, Device::kApIp4),
        IPAddress(255, 255, 255, 0));
    WiFi.softAP(SETUP_AP_SSID);

    dnsServer_.start(Device::kDnsPort, "*", WiFi.softAPIP());
    setupRoutes();
    server_.begin();

    active_ = true;
    complete_ = false;

    eventBus_.publish(makeDisplayStatusEvent(Ui::kSetupLine1, Ui::kSetupLine2));
}

void WiFiProvisioner::loop() {
    if (!active_) {
        return;
    }

    dnsServer_.processNextRequest();
    server_.handleClient();
}

bool WiFiProvisioner::isComplete() const {
    return complete_;
}

bool WiFiProvisioner::isActive() const {
    return active_;
}

void WiFiProvisioner::setupRoutes() {
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/setup.html", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/css/style.css", HTTP_GET,
               [this]() { files_.serve("/web/css/style.css", "text/css"); });
    server_.on("/js/constants.js", HTTP_GET,
               [this]() { files_.serve("/web/js/constants.js", "application/javascript"); });
    server_.on("/js/ui.js", HTTP_GET,
               [this]() { files_.serve("/web/js/ui.js", "application/javascript"); });
    server_.on("/js/api.js", HTTP_GET,
               [this]() { files_.serve("/web/js/api.js", "application/javascript"); });
    server_.on("/js/setup.js", HTTP_GET,
               [this]() { files_.serve("/web/js/setup.js", "application/javascript"); });
    server_.on("/api/setup/scan", HTTP_GET, [this]() { handleScan(); });
    server_.on("/api/setup/connect", HTTP_POST, [this]() { handleConnect(); });
    server_.on("/generate_204", HTTP_ANY, [this]() { handleCaptiveProbe(); });
    server_.on("/hotspot-detect.html", HTTP_ANY, [this]() { handleCaptiveProbe(); });
    server_.on("/connecttest.txt", HTTP_ANY, [this]() { handleCaptiveProbe(); });
    server_.on("/fwlink", HTTP_ANY, [this]() { handleCaptiveProbe(); });
    server_.onNotFound([this]() {
        if (captivePortal()) {
            return;
        }
        files_.sendNotFound();
    });
}

bool WiFiProvisioner::captivePortal() {
    server_.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    server_.send(302, "text/plain", "");
    return true;
}

void WiFiProvisioner::handleRoot() {
    if (files_.serve("/web/setup.html", "text/html")) {
        return;
    }

    server_.send(200, "text/html",
                 "<html><body><h1>Vorlaxen Setup</h1>"
                 "<p>setup.html not found. Upload LittleFS.</p></body></html>");
}

void WiFiProvisioner::handleScan() {
    const int networkCount = WiFi.scanNetworks(false, true);

    JsonDocument doc;
    JsonArray networks = doc["networks"].to<JsonArray>();

    for (int i = 0; i < networkCount; i++) {
        JsonObject entry = networks.add<JsonObject>();
        entry["ssid"] = WiFi.SSID(i);
        entry["rssi"] = WiFi.RSSI(i);
        entry["secure"] = WiFi.encryptionType(i) != ENC_TYPE_NONE;
    }

    String response;
    serializeJson(doc, response);
    server_.send(200, "application/json", response);
}

void WiFiProvisioner::handleConnect() {
    if (!server_.hasArg("plain")) {
        JsonResponse::sendError(server_, 400, "invalid_body");
        return;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, server_.arg("plain"));
    if (error) {
        JsonResponse::sendError(server_, 400, "invalid_json");
        return;
    }

    const char* ssid = doc["ssid"] | "";
    const char* password = doc["password"] | "";

    if (strlen(ssid) == 0) {
        JsonResponse::sendError(server_, 400, "ssid_required");
        return;
    }

    config_.saveWiFi(String(ssid), String(password));
    JsonResponse::sendOk(server_);

    delay(250);

    active_ = false;
    complete_ = true;
}

void WiFiProvisioner::handleCaptiveProbe() {
    captivePortal();
}
