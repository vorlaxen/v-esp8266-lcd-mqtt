#pragma once

#include "config/ConfigTypes.h"
#include "core/IModule.h"
#include "core/DeviceConstants.h"
#include <Arduino.h>

#ifndef MQTT_DEFAULT_PORT
#define MQTT_DEFAULT_PORT 1883
#endif

#ifndef MQTT_TOPIC_PREFIX
#define MQTT_TOPIC_PREFIX "vorlaxen"
#endif

struct MqttSettings {
    bool enabled = false;
    String broker;
    uint16_t port = MQTT_DEFAULT_PORT;
    String username;
    String password;
    String topicPrefix = MQTT_TOPIC_PREFIX;
};

class ConfigModule : public IModule {
public:
    void init() override;
    void loop() override {}

    bool hasWiFiCredentials() const;
    bool loadWiFi(String& ssid, String& password) const;
    void saveWiFi(const String& ssid, const String& password);

    String getDeviceId() const;
    String getMdnsName() const;
    String getHostname() const;

    bool isMqttEnabled() const;
    bool hasMqttBroker() const;
    String getMqttBroker() const;
    uint16_t getMqttPort() const;
    String getMqttUsername() const;
    String getMqttPassword() const;
    String getMqttTopicPrefix() const;
    MqttSettings getMqttSettings() const;
    void saveMqttSettings(const MqttSettings& settings);

    void clear();
    void clearWiFi();
    void logBootStatus() const;

private:
    static constexpr uint8_t kMagic = CONFIG_MAGIC;
    static constexpr uint8_t kVersion = CONFIG_VERSION;
    static constexpr size_t kEepromSize = CONFIG_EEPROM_SIZE;

    ConfigData data_{};
    bool valid_ = false;

    void load();
    void persist();
    void ensureDeviceId();
    void applyMqttDefaults();
    void resetToDefaults();
    void migrateFromV1(const ConfigDataV1& old);
    void migrateFromV2Legacy(const ConfigDataV2Legacy& old);
};
