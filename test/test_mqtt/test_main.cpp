#include <unity.h>
#include "mqtt/MqttTopics.h"
#include "mqtt/MqttMessageParser.h"
#include <string.h>

static MqttTopicContext makeCtx() {
    return {"vorlaxen", "123456"};
}

void test_topic_display_set() {
    char topic[64];
    const MqttTopicContext ctx = makeCtx();
    TEST_ASSERT_GREATER_THAN(0, mqttTopicDisplaySet(topic, sizeof(topic), &ctx));
    TEST_ASSERT_EQUAL_STRING("vorlaxen/123456/display/set", topic);
}

void test_topic_display_restore() {
    char topic[64];
    const MqttTopicContext ctx = makeCtx();
    TEST_ASSERT_GREATER_THAN(0, mqttTopicDisplayRestore(topic, sizeof(topic), &ctx));
    TEST_ASSERT_EQUAL_STRING("vorlaxen/123456/display/restore", topic);
}

void test_topic_status() {
    char topic[64];
    const MqttTopicContext ctx = makeCtx();
    TEST_ASSERT_EQUAL(22, mqttTopicStatus(topic, sizeof(topic), &ctx));
    TEST_ASSERT_EQUAL_STRING("vorlaxen/123456/status", topic);
}

void test_topic_availability() {
    char topic[64];
    const MqttTopicContext ctx = makeCtx();
    TEST_ASSERT_EQUAL(28, mqttTopicAvailability(topic, sizeof(topic), &ctx));
    TEST_ASSERT_EQUAL_STRING("vorlaxen/123456/availability", topic);
}

void test_topic_null_prefix() {
    char topic[64];
    const MqttTopicContext ctx = {"vorlaxen", nullptr};
    TEST_ASSERT_EQUAL(-1, mqttTopicDisplaySet(topic, sizeof(topic), &ctx));
}

void test_topic_buffer_too_small() {
    char topic[8];
    const MqttTopicContext ctx = makeCtx();
    TEST_ASSERT_EQUAL(-1, mqttTopicDisplaySet(topic, sizeof(topic), &ctx));
}

void test_topic_null_context() {
    char topic[64];
    TEST_ASSERT_EQUAL(-1, mqttTopicDisplaySet(topic, sizeof(topic), nullptr));
}

void test_parse_plain_text() {
    const char* payload = "Hello MQTT";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL_STRING("Hello MQTT", msg.text);
    TEST_ASSERT_TRUE(msg.scroll);
    TEST_ASSERT_EQUAL(300, msg.scrollMs);
}

void test_parse_json_display() {
    const char* payload = R"({"text":"Merhaba","scroll":false,"scrollMs":500})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL_STRING("Merhaba", msg.text);
    TEST_ASSERT_FALSE(msg.scroll);
    TEST_ASSERT_EQUAL(500, msg.scrollMs);
}

void test_parse_json_message_alias() {
    const char* payload = R"({"message":"Alias field"})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL_STRING("Alias field", msg.text);
}

void test_parse_restore_action() {
    const char* payload = R"({"action":"restore"})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_empty_restore() {
    const MqttParsedMessage msg = mqttParsePayload("", 0);
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_invalid_json() {
    const char* payload = "{not json";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_INVALID, msg.action);
}

void test_parse_truncates_long_text() {
    char payload[80];
    memset(payload, 'A', 70);
    payload[70] = '\0';
    const MqttParsedMessage msg = mqttParsePayload(payload, 70);
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL(MQTT_PARSED_TEXT_MAX, strlen(msg.text));
}

void test_parse_whitespace_only_is_restore() {
    const char* payload = "   \t  ";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_json_empty_text_restore() {
    const char* payload = R"({"text":""})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_scroll_ms_clamped() {
    const char* payload = R"({"text":"Fast","scrollMs":10})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL(300, msg.scrollMs);
}

void test_parse_turkish_text() {
    const char* payload = R"({"text":"Merhaba Dünya","scroll":false})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL_STRING("Merhaba Dünya", msg.text);
}

void test_parse_null_payload() {
    const MqttParsedMessage msg = mqttParsePayload(nullptr, 0);
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_whitespace_only_json_restore() {
    const char* payload = "   ";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_topic_empty_device_id() {
    char topic[64];
    const MqttTopicContext ctx = {"vorlaxen", ""};
    TEST_ASSERT_EQUAL(-1, mqttTopicDisplaySet(topic, sizeof(topic), &ctx));
}

void test_parse_json_numeric_text_rejected() {
    const char* payload = R"({"text":12345})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_RESTORE, msg.action);
}

void test_parse_leading_trailing_spaces_trimmed() {
    const char* payload = R"({"text":"  padded  "})";
    const MqttParsedMessage msg = mqttParsePayload(payload, strlen(payload));
    TEST_ASSERT_EQUAL(MQTT_ACTION_DISPLAY, msg.action);
    TEST_ASSERT_EQUAL_STRING("padded", msg.text);
}

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_topic_display_set);
    RUN_TEST(test_topic_display_restore);
    RUN_TEST(test_topic_status);
    RUN_TEST(test_topic_availability);
    RUN_TEST(test_topic_buffer_too_small);
    RUN_TEST(test_topic_null_context);
    RUN_TEST(test_topic_null_prefix);
    RUN_TEST(test_topic_empty_device_id);
    RUN_TEST(test_parse_plain_text);
    RUN_TEST(test_parse_json_display);
    RUN_TEST(test_parse_json_message_alias);
    RUN_TEST(test_parse_restore_action);
    RUN_TEST(test_parse_empty_restore);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_parse_truncates_long_text);
    RUN_TEST(test_parse_whitespace_only_is_restore);
    RUN_TEST(test_parse_json_empty_text_restore);
    RUN_TEST(test_parse_scroll_ms_clamped);
    RUN_TEST(test_parse_turkish_text);
    RUN_TEST(test_parse_null_payload);
    RUN_TEST(test_parse_whitespace_only_json_restore);
    RUN_TEST(test_parse_json_numeric_text_rejected);
    RUN_TEST(test_parse_leading_trailing_spaces_trimmed);
    return UNITY_END();
}
