#include "mqtt/MqttTopics.h"
#include <stdio.h>
#include <string.h>

extern "C" {

static int buildTopic(char* out, size_t outLen, const MqttTopicContext* ctx, const char* suffix) {
    if (!out || !ctx || !ctx->prefix || !ctx->deviceId || !suffix) {
        return -1;
    }
    if (ctx->prefix[0] == '\0' || ctx->deviceId[0] == '\0') {
        return -1;
    }

    const int written = snprintf(out, outLen, "%s/%s/%s", ctx->prefix, ctx->deviceId, suffix);
    if (written < 0 || static_cast<size_t>(written) >= outLen) {
        return -1;
    }

    return written;
}

int mqttTopicDisplaySet(char* out, size_t outLen, const MqttTopicContext* ctx) {
    return buildTopic(out, outLen, ctx, "display/set");
}

int mqttTopicDisplayRestore(char* out, size_t outLen, const MqttTopicContext* ctx) {
    return buildTopic(out, outLen, ctx, "display/restore");
}

int mqttTopicStatus(char* out, size_t outLen, const MqttTopicContext* ctx) {
    return buildTopic(out, outLen, ctx, "status");
}

int mqttTopicAvailability(char* out, size_t outLen, const MqttTopicContext* ctx) {
    return buildTopic(out, outLen, ctx, "availability");
}

} // extern "C"
