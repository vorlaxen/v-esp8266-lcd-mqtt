#pragma once

#include <Arduino.h>

#ifndef HOSTNAME_PREFIX
#define HOSTNAME_PREFIX "v"
#endif

#ifndef SETUP_AP_SSID
#define SETUP_AP_SSID "VESPSetup"
#endif

#ifndef FACTORY_RESET_PIN
#define FACTORY_RESET_PIN D5
#endif

namespace Device {

constexpr uint8_t kLcdAddress = 0x27;
constexpr uint8_t kLcdCols = 16;
constexpr uint8_t kLcdRows = 2;
constexpr uint8_t kLcdSdaPin = D2;
constexpr uint8_t kLcdSclPin = D1;

constexpr size_t kMaxMessageLen = 64;
constexpr uint16_t kDefaultScrollDelayMs = 300;
constexpr uint16_t kMinScrollDelayMs = 50;

constexpr unsigned long kConnectTimeoutMs = 20000UL;
constexpr unsigned long kNoSsidFailMs = 15000UL;
constexpr unsigned long kReconnectIntervalMs = 10000UL;
constexpr unsigned long kStatusMessageHoldMs = 1500UL;

constexpr unsigned long kFactoryResetHoldMs = 500UL;
constexpr unsigned long kResetMessageHoldMs = 1500UL;
constexpr unsigned long kFactoryResetGraceMs = 3000UL;

constexpr uint8_t kApIp = 192;
constexpr uint8_t kApIp2 = 168;
constexpr uint8_t kApIp3 = 4;
constexpr uint8_t kApIp4 = 1;

constexpr uint16_t kHttpPort = 80;
constexpr uint16_t kDnsPort = 53;

constexpr uint32_t kSerialBaud = 115200;
constexpr unsigned long kBootDelayMs = 500UL;

constexpr unsigned long kStatusRefreshIntervalMs = 8000UL;
constexpr unsigned long kSetupScanIntervalMs = 15000UL;

constexpr uint16_t kMqttDefaultPort = 1883;
constexpr size_t kMqttBufferSize = 512;
constexpr unsigned long kMqttReconnectMinMs = 3000UL;
constexpr unsigned long kMqttReconnectMaxMs = 60000UL;
constexpr unsigned long kMqttStatusIntervalMs = 30000UL;

} // namespace Device
