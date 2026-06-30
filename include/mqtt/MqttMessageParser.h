#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_PARSED_TEXT_MAX 64

enum MqttParsedAction {
    MQTT_ACTION_INVALID = 0,
    MQTT_ACTION_DISPLAY,
    MQTT_ACTION_RESTORE
};

struct MqttParsedMessage {
    enum MqttParsedAction action;
    char text[MQTT_PARSED_TEXT_MAX + 1];
    bool scroll;
    uint16_t scrollMs;
};
/**
 * Parse an MQTT payload into a display command.
 * Accepts plain text or JSON: {"text":"...","scroll":true,"scrollMs":300}
 * Empty payload or "restore" JSON action → MQTT_ACTION_RESTORE.
 */
struct MqttParsedMessage mqttParsePayload(const char* payload, size_t length);

#ifdef __cplusplus
}
#endif
