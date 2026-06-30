#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_MAGIC 0xA5U
#define CONFIG_VERSION 3U
#define CONFIG_EEPROM_SIZE 512U

struct __attribute__((packed)) ConfigDataV1 {
    uint8_t magic;
    uint8_t version;
    char deviceId[8];
    char ssid[32];
    char password[64];
    uint8_t checksum;
};

/** v2 EEPROM layout (not packed — matches deployed firmware v2 padding). */
struct ConfigDataV2Legacy {
    uint8_t magic;
    uint8_t version;
    char deviceId[8];
    char ssid[32];
    char password[64];
    uint8_t mqttEnabled;
    char mqttBroker[64];
    uint16_t mqttPort;
    char mqttUser[32];
    char mqttPassword[64];
    char mqttTopicPrefix[32];
    uint8_t checksum;
};

struct __attribute__((packed)) ConfigData {
    uint8_t magic;
    uint8_t version;
    char deviceId[8];
    char ssid[32];
    char password[64];
    uint8_t mqttEnabled;
    char mqttBroker[64];
    uint16_t mqttPort;
    char mqttUser[32];
    char mqttPassword[64];
    char mqttTopicPrefix[32];
    uint8_t checksum;
};

uint8_t configComputeChecksum(const uint8_t* data, size_t length);
uint8_t configComputeChecksumStruct(const struct ConfigData* data);
uint8_t configComputeChecksumV1(const struct ConfigDataV1* data);
bool configValidateChecksumStruct(const struct ConfigData* data);
bool configValidateChecksumV1(const struct ConfigDataV1* data);

#ifdef __cplusplus
}
#endif
