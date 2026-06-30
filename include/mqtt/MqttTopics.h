#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MqttTopicContext {
    const char* prefix;
    const char* deviceId;
};

/** Returns bytes written, or -1 if buffer too small. */
int mqttTopicDisplaySet(char* out, size_t outLen, const MqttTopicContext* ctx);
int mqttTopicDisplayRestore(char* out, size_t outLen, const MqttTopicContext* ctx);
int mqttTopicStatus(char* out, size_t outLen, const MqttTopicContext* ctx);
int mqttTopicAvailability(char* out, size_t outLen, const MqttTopicContext* ctx);

#ifdef __cplusplus
}
#endif
