#include "network/NetworkModule.h"
#include "network/WiFiProvisioner.h"
#include "core/UiStrings.h"
#include "core/DeviceConstants.h"
#include "core/Log.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <memory>

NetworkModule::NetworkModule(ConfigModule& config, EventBus& eventBus)
    : config_(config), eventBus_(eventBus) {}

void NetworkModule::init() {
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);

    if (config_.hasWiFiCredentials()) {
        Log::info("Network: using saved WiFi");
        startConnecting();
    } else {
        Log::info("Network: opening VESPSetup");
        startProvisioning();
    }
}

void NetworkModule::loop() {
    if (state_ == State::Provisioning && provisioner_) {
        provisioner_->loop();

        if (provisioner_->isComplete()) {
            WiFi.softAPdisconnect(true);
            provisioner_.reset();
            startConnecting();
        }
        return;
    }

    if (state_ == State::Connecting || state_ == State::Disconnected) {
        const wl_status_t wifiStatus = WiFi.status();

        if (wifiStatus == WL_CONNECTED) {
            handleConnected();
            return;
        }

        if (state_ == State::Connecting) {
            const unsigned long elapsed = millis() - connectStartedAt_;

            if (wifiStatus == WL_NO_SSID_AVAIL && elapsed >= Device::kNoSsidFailMs) {
                handleConnectFailed();
                return;
            }

            if (elapsed >= Device::kConnectTimeoutMs) {
                handleConnectFailed();
                return;
            }
        }

        if (state_ == State::Disconnected &&
            millis() - lastReconnectAttempt_ > Device::kReconnectIntervalMs) {
            startConnecting();
        }
    }

    if (state_ == State::Connected) {
        MDNS.update();

        if (WiFi.status() != WL_CONNECTED) {
            handleDisconnected();
        }
    }
}

bool NetworkModule::isConnected() const {
    return state_ == State::Connected;
}

String NetworkModule::getIp() const {
    return WiFi.localIP().toString();
}

String NetworkModule::getHostname() const {
    return config_.getHostname();
}

String NetworkModule::getSsid() const {
    return WiFi.SSID();
}

int32_t NetworkModule::getRssi() const {
    return WiFi.RSSI();
}

void NetworkModule::resetToProvisioning() {
    WiFi.disconnect(true);
    MDNS.close();
    provisioner_.reset();
    state_ = State::Idle;
    startProvisioning();
}

void NetworkModule::startProvisioning() {
    state_ = State::Provisioning;

    if (!provisioner_) {
        provisioner_ = std::make_unique<WiFiProvisioner>(config_, eventBus_);
    }

    provisioner_->start();
}

void NetworkModule::startConnecting() {
    state_ = State::Connecting;
    lastReconnectAttempt_ = millis();
    connectStartedAt_ = millis();

    eventBus_.publish(makeDisplayStatusEvent(Ui::kConnecting, Ui::kPleaseWait));

    String ssid;
    String password;
    if (!config_.loadWiFi(ssid, password)) {
        startProvisioning();
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
}

void NetworkModule::handleConnected() {
    if (state_ == State::Connected) {
        return;
    }

    state_ = State::Connected;

    const String ip = WiFi.localIP().toString();
    const String hostname = config_.getHostname();
    const String mdnsName = config_.getMdnsName();

    if (MDNS.begin(mdnsName.c_str())) {
        MDNS.addService("http", "tcp", Device::kHttpPort);
    }

    Log::infof("Network: connected %s", ip.c_str());

    eventBus_.publish(
        makeNetworkConnectedEvent(ip, hostname, config_.getDeviceId()));
}

void NetworkModule::handleDisconnected() {
    if (state_ == State::Disconnected) {
        return;
    }

    state_ = State::Disconnected;
    lastReconnectAttempt_ = millis();
    MDNS.close();

    eventBus_.publish(makeNetworkDisconnectedEvent());
}

void NetworkModule::handleConnectFailed() {
    WiFi.disconnect();
    state_ = State::Disconnected;
    lastReconnectAttempt_ = millis();

    eventBus_.publish(makeDisplayStatusEvent(Ui::kConnectionLost, Ui::kRetrying));
}
