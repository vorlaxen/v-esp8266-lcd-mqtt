#include "mqtt/MqttMessageParser.h"
#include <ArduinoJson.h>
#include <ctype.h>
#include <string.h>

#ifndef MQTT_DEFAULT_SCROLL_MS
#define MQTT_DEFAULT_SCROLL_MS 300
#endif

#ifndef MQTT_MIN_SCROLL_MS
#define MQTT_MIN_SCROLL_MS 50
#endif

extern "C" {

static void setInvalid(struct MqttParsedMessage* msg) {
    msg->action = MQTT_ACTION_INVALID;
    msg->text[0] = '\0';
    msg->scroll = true;
    msg->scrollMs = MQTT_DEFAULT_SCROLL_MS;
}

static void setRestore(struct MqttParsedMessage* msg) {
    msg->action = MQTT_ACTION_RESTORE;
    msg->text[0] = '\0';
}

static void trimCopyText(struct MqttParsedMessage* msg, const char* source) {
    if (!source) {
        msg->text[0] = '\0';
        return;
    }

    while (*source && isspace(static_cast<unsigned char>(*source))) {
        source++;
    }

    size_t len = strlen(source);
    while (len > 0 && isspace(static_cast<unsigned char>(source[len - 1]))) {
        len--;
    }

    if (len > MQTT_PARSED_TEXT_MAX) {
        len = MQTT_PARSED_TEXT_MAX;
    }

    memcpy(msg->text, source, len);
    msg->text[len] = '\0';
}

struct MqttParsedMessage mqttParsePayload(const char* payload, size_t length) {
    struct MqttParsedMessage msg{};
    msg.scroll = true;
    msg.scrollMs = MQTT_DEFAULT_SCROLL_MS;

    if (!payload || length == 0) {
        setRestore(&msg);
        return msg;
    }

    while (length > 0 && isspace(static_cast<unsigned char>(payload[length - 1]))) {
        length--;
    }

    if (length == 0) {
        setRestore(&msg);
        return msg;
    }

    if (payload[0] != '{') {
        msg.action = MQTT_ACTION_DISPLAY;
        char buffer[MQTT_PARSED_TEXT_MAX + 1];
        const size_t copyLen = length > MQTT_PARSED_TEXT_MAX ? MQTT_PARSED_TEXT_MAX : length;
        memcpy(buffer, payload, copyLen);
        buffer[copyLen] = '\0';
        trimCopyText(&msg, buffer);
        if (msg.text[0] == '\0') {
            setInvalid(&msg);
        }
        return msg;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        setInvalid(&msg);
        return msg;
    }

    const char* action = doc["action"] | "";
    if (strcmp(action, "restore") == 0) {
        setRestore(&msg);
        return msg;
    }

    const char* text = doc["text"] | doc["message"] | "";
    if (text[0] == '\0') {
        setRestore(&msg);
        return msg;
    }

    msg.action = MQTT_ACTION_DISPLAY;
    trimCopyText(&msg, text);
    msg.scroll = doc["scroll"] | true;
    msg.scrollMs = doc["scrollMs"] | MQTT_DEFAULT_SCROLL_MS;
    if (msg.scrollMs < MQTT_MIN_SCROLL_MS) {
        msg.scrollMs = MQTT_DEFAULT_SCROLL_MS;
    }

    return msg;
}

} // extern "C"
