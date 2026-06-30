#include <unity.h>
#include "config/ConfigModule.h"
#include "config/ConfigTypes.h"
#include <EEPROM.h>
#include <string.h>

static void writeV3Config(const char* deviceId, const char* ssid, const char* password) {
    ConfigData data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = CONFIG_VERSION;
    strncpy(data.deviceId, deviceId, sizeof(data.deviceId));
    strncpy(data.ssid, ssid, sizeof(data.ssid));
    strncpy(data.password, password, sizeof(data.password));
    data.mqttPort = MQTT_DEFAULT_PORT;
    strncpy(data.mqttTopicPrefix, MQTT_TOPIC_PREFIX, sizeof(data.mqttTopicPrefix));
    data.checksum = configComputeChecksumStruct(&data);
    EEPROM.put(0, data);
    EEPROM.commit();
}

static void writeV1Config(const char* deviceId, const char* ssid) {
    ConfigDataV1 data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = 1;
    strncpy(data.deviceId, deviceId, sizeof(data.deviceId));
    strncpy(data.ssid, ssid, sizeof(data.ssid));
    data.checksum = configComputeChecksumV1(&data);
    EEPROM.put(0, data);
    EEPROM.commit();
}

void setUp() {
    EEPROM.reset();
    EEPROM.begin(CONFIG_EEPROM_SIZE);
    ESP.setChipId(134297);
}

void tearDown() {}

void test_save_and_load_wifi_via_module() {
    ConfigModule config;
    config.init();
    config.saveWiFi("MyWiFi", "secret123");

    ConfigModule reloaded;
    reloaded.init();

    String ssid;
    String password;
    TEST_ASSERT_TRUE(reloaded.loadWiFi(ssid, password));
    TEST_ASSERT_EQUAL_STRING("MyWiFi", ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret123", password.c_str());
}

void test_load_valid_wifi_credentials() {
    writeV3Config("134297", "MyWiFi", "secret123");

    ConfigData stored{};
    EEPROM.get(0, stored);
    TEST_ASSERT_TRUE(configValidateChecksumStruct(&stored));

    ConfigModule config;
    config.init();

    String ssid;
    String password;
    TEST_ASSERT_TRUE(config.loadWiFi(ssid, password));
    TEST_ASSERT_EQUAL_STRING("MyWiFi", ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret123", password.c_str());
}

void test_hostname_format() {
    writeV3Config("134297", "MyWiFi", "secret123");

    ConfigModule config;
    config.init();

    TEST_ASSERT_EQUAL_STRING("v-134297", config.getMdnsName().c_str());
    TEST_ASSERT_EQUAL_STRING("v-134297.local", config.getHostname().c_str());
}

void test_generates_device_id_when_missing() {
    ConfigData data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = CONFIG_VERSION;
    data.checksum = configComputeChecksumStruct(&data);
    EEPROM.put(0, data);
    EEPROM.commit();

    ConfigModule config;
    config.init();

    TEST_ASSERT_EQUAL_STRING("134297", config.getDeviceId().c_str());
}

void test_migrate_v1_to_v3() {
    writeV1Config("111111", "LegacyNet");

    ConfigDataV1 stored{};
    EEPROM.get(0, stored);
    TEST_ASSERT_TRUE(configValidateChecksumV1(&stored));

    ConfigModule config;
    config.init();

    String ssid;
    String password;
    TEST_ASSERT_TRUE(config.loadWiFi(ssid, password));
    TEST_ASSERT_EQUAL_STRING("LegacyNet", ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("111111", config.getDeviceId().c_str());
}

void test_save_mqtt_settings_roundtrip() {
    writeV3Config("134297", "MyWiFi", "secret123");

    ConfigModule config;
    config.init();

    MqttSettings settings;
    settings.enabled = true;
    settings.broker = "10.0.0.5";
    settings.port = 1883;
    settings.username = "user";
    settings.password = "pass";
    settings.topicPrefix = "vorlaxen";
    config.saveMqttSettings(settings);

    const MqttSettings loaded = config.getMqttSettings();
    TEST_ASSERT_TRUE(loaded.enabled);
    TEST_ASSERT_EQUAL_STRING("10.0.0.5", loaded.broker.c_str());
    TEST_ASSERT_EQUAL(1883, loaded.port);
    TEST_ASSERT_EQUAL_STRING("user", loaded.username.c_str());
    TEST_ASSERT_EQUAL_STRING("pass", loaded.password.c_str());
}

void test_clear_wifi_keeps_device_id() {
    writeV3Config("134297", "MyWiFi", "secret123");

    ConfigModule config;
    config.init();
    config.clearWiFi();

    String ssid;
    String password;
    TEST_ASSERT_FALSE(config.loadWiFi(ssid, password));
    TEST_ASSERT_EQUAL_STRING("134297", config.getDeviceId().c_str());
}

void test_invalid_checksum_resets_to_defaults() {
    ConfigData data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = CONFIG_VERSION;
    strncpy(data.deviceId, "999999", sizeof(data.deviceId));
    data.checksum = 0xAB;
    EEPROM.put(0, data);
    EEPROM.commit();

    ConfigModule config;
    config.init();

    TEST_ASSERT_EQUAL_STRING("134297", config.getDeviceId().c_str());
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_save_and_load_wifi_via_module);
    RUN_TEST(test_load_valid_wifi_credentials);
    RUN_TEST(test_hostname_format);
    RUN_TEST(test_generates_device_id_when_missing);
    RUN_TEST(test_migrate_v1_to_v3);
    RUN_TEST(test_save_mqtt_settings_roundtrip);
    RUN_TEST(test_clear_wifi_keeps_device_id);
    RUN_TEST(test_invalid_checksum_resets_to_defaults);
    return UNITY_END();
}
