#include "config/ConfigModule.h"
#include "config/ConfigTypes.h"
#include "core/Log.h"
#include <EEPROM.h>
#include <string.h>

#ifndef MQTT_DEFAULT_BROKER
#define MQTT_DEFAULT_BROKER ""
#endif

void ConfigModule::init() {
    EEPROM.begin(kEepromSize);
    load();
    ensureDeviceId();
    logBootStatus();
}

void ConfigModule::logBootStatus() const {
    Log::infof("Config v%d valid=%s device=%s",
               static_cast<int>(data_.version),
               valid_ ? "yes" : "no",
               data_.deviceId);

    if (hasWiFiCredentials()) {
        Log::infof("WiFi saved: %s", data_.ssid);
    } else {
        Log::info("WiFi saved: (none) -> VESPSetup");
    }

    if (isMqttEnabled() && hasMqttBroker()) {
        Log::infof("MQTT enabled: %s:%u", data_.mqttBroker, getMqttPort());
    }
}

bool ConfigModule::hasWiFiCredentials() const {
    return valid_ && data_.ssid[0] != '\0';
}

bool ConfigModule::loadWiFi(String& ssid, String& password) const {
    if (!hasWiFiCredentials()) {
        return false;
    }

    ssid = String(data_.ssid);
    password = String(data_.password);
    return true;
}

void ConfigModule::saveWiFi(const String& ssid, const String& password) {
    ssid.toCharArray(data_.ssid, sizeof(data_.ssid));
    password.toCharArray(data_.password, sizeof(data_.password));
    data_.magic = kMagic;
    data_.version = kVersion;
    persist();
}

String ConfigModule::getDeviceId() const {
    return String(data_.deviceId);
}

String ConfigModule::getMdnsName() const {
    return String(HOSTNAME_PREFIX) + "-" + getDeviceId();
}

String ConfigModule::getHostname() const {
    return getMdnsName() + ".local";
}

bool ConfigModule::isMqttEnabled() const {
    return data_.mqttEnabled != 0;
}

bool ConfigModule::hasMqttBroker() const {
    return data_.mqttBroker[0] != '\0';
}

String ConfigModule::getMqttBroker() const {
    return String(data_.mqttBroker);
}

uint16_t ConfigModule::getMqttPort() const {
    return data_.mqttPort == 0 ? MQTT_DEFAULT_PORT : data_.mqttPort;
}

String ConfigModule::getMqttUsername() const {
    return String(data_.mqttUser);
}

String ConfigModule::getMqttPassword() const {
    return String(data_.mqttPassword);
}

String ConfigModule::getMqttTopicPrefix() const {
    return data_.mqttTopicPrefix[0] != '\0' ? String(data_.mqttTopicPrefix)
                                             : String(MQTT_TOPIC_PREFIX);
}

MqttSettings ConfigModule::getMqttSettings() const {
    MqttSettings settings;
    settings.enabled = isMqttEnabled();
    settings.broker = getMqttBroker();
    settings.port = getMqttPort();
    settings.username = getMqttUsername();
    settings.password = getMqttPassword();
    settings.topicPrefix = getMqttTopicPrefix();
    return settings;
}

void ConfigModule::saveMqttSettings(const MqttSettings& settings) {
    data_.mqttEnabled = settings.enabled ? 1 : 0;
    settings.broker.toCharArray(data_.mqttBroker, sizeof(data_.mqttBroker));
    data_.mqttPort = settings.port == 0 ? MQTT_DEFAULT_PORT : settings.port;
    settings.username.toCharArray(data_.mqttUser, sizeof(data_.mqttUser));
    settings.password.toCharArray(data_.mqttPassword, sizeof(data_.mqttPassword));
    settings.topicPrefix.toCharArray(data_.mqttTopicPrefix, sizeof(data_.mqttTopicPrefix));
    data_.magic = kMagic;
    data_.version = kVersion;
    persist();
}

void ConfigModule::clear() {
    resetToDefaults();
    ensureDeviceId();
    persist();
}

void ConfigModule::clearWiFi() {
    data_.ssid[0] = '\0';
    data_.password[0] = '\0';
    persist();
}

void ConfigModule::applyMqttDefaults() {
    data_.mqttEnabled = 0;
    data_.mqttBroker[0] = '\0';
    data_.mqttPort = MQTT_DEFAULT_PORT;
    data_.mqttUser[0] = '\0';
    data_.mqttPassword[0] = '\0';
    strncpy(data_.mqttTopicPrefix, MQTT_TOPIC_PREFIX, sizeof(data_.mqttTopicPrefix) - 1);

#ifdef MQTT_DEFAULT_BROKER
    if (MQTT_DEFAULT_BROKER[0] != '\0') {
        strncpy(data_.mqttBroker, MQTT_DEFAULT_BROKER, sizeof(data_.mqttBroker) - 1);
        data_.mqttEnabled = 1;
    }
#endif
}

void ConfigModule::resetToDefaults() {
    memset(&data_, 0, sizeof(data_));
    data_.magic = kMagic;
    data_.version = kVersion;
    applyMqttDefaults();
    valid_ = false;
}

void ConfigModule::migrateFromV1(const ConfigDataV1& old) {
    resetToDefaults();
    strncpy(data_.deviceId, old.deviceId, sizeof(data_.deviceId) - 1);
    strncpy(data_.ssid, old.ssid, sizeof(data_.ssid) - 1);
    strncpy(data_.password, old.password, sizeof(data_.password) - 1);
}

void ConfigModule::migrateFromV2Legacy(const ConfigDataV2Legacy& old) {
    resetToDefaults();
    strncpy(data_.deviceId, old.deviceId, sizeof(data_.deviceId) - 1);
    strncpy(data_.ssid, old.ssid, sizeof(data_.ssid) - 1);
    strncpy(data_.password, old.password, sizeof(data_.password) - 1);
    data_.mqttEnabled = old.mqttEnabled;
    strncpy(data_.mqttBroker, old.mqttBroker, sizeof(data_.mqttBroker) - 1);
    data_.mqttPort = old.mqttPort == 0 ? MQTT_DEFAULT_PORT : old.mqttPort;
    strncpy(data_.mqttUser, old.mqttUser, sizeof(data_.mqttUser) - 1);
    strncpy(data_.mqttPassword, old.mqttPassword, sizeof(data_.mqttPassword) - 1);
    strncpy(data_.mqttTopicPrefix, old.mqttTopicPrefix, sizeof(data_.mqttTopicPrefix) - 1);
    if (data_.mqttTopicPrefix[0] == '\0') {
        strncpy(data_.mqttTopicPrefix, MQTT_TOPIC_PREFIX, sizeof(data_.mqttTopicPrefix) - 1);
    }
}

void ConfigModule::load() {
    ConfigData stored{};
    EEPROM.get(0, stored);

    if (stored.magic == kMagic && stored.version == kVersion && configValidateChecksumStruct(&stored)) {
        data_ = stored;
        valid_ = true;
        return;
    }

    ConfigDataV2Legacy oldV2{};
    EEPROM.get(0, oldV2);
    if (oldV2.magic == kMagic && oldV2.version == 2) {
        migrateFromV2Legacy(oldV2);
        Log::info("Config migrated v2->v3");
        persist();
        return;
    }

    ConfigDataV1 oldV1{};
    EEPROM.get(0, oldV1);
    if (oldV1.magic == kMagic && oldV1.version == 1 && configValidateChecksumV1(&oldV1)) {
            migrateFromV1(oldV1);
            Log::info("Config migrated v1->v3");
            persist();
            return;
    }

    Log::warn("Config invalid, using defaults");
    resetToDefaults();
    ensureDeviceId();
    persist();
}

void ConfigModule::persist() {
    data_.magic = kMagic;
    data_.version = kVersion;
    data_.checksum = 0;
    data_.checksum = configComputeChecksumStruct(&data_);

    EEPROM.put(0, data_);
    if (!EEPROM.commit()) {
        Log::error("EEPROM commit failed");
        valid_ = false;
        return;
    }

    valid_ = true;
}

void ConfigModule::ensureDeviceId() {
    if (data_.deviceId[0] != '\0') {
        return;
    }

    const uint32_t chipId = ESP.getChipId() % 1000000UL;
    snprintf(data_.deviceId, sizeof(data_.deviceId), "%06lu", static_cast<unsigned long>(chipId));
}
