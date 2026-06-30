#include <unity.h>
#include "core/Events.h"
#include "core/DeviceConstants.h"

void test_display_status_event_fields() {
    const Event event = makeDisplayStatusEvent("Line1", "Line2");
    TEST_ASSERT_EQUAL(EventType::DisplayStatus, event.type);
    TEST_ASSERT_EQUAL_STRING("Line1", event.line1.c_str());
    TEST_ASSERT_EQUAL_STRING("Line2", event.line2.c_str());
}

void test_network_connected_event_fields() {
    const Event event = makeNetworkConnectedEvent("192.168.1.4", "v-134297.local", "134297");
    TEST_ASSERT_EQUAL(EventType::NetworkConnected, event.type);
    TEST_ASSERT_EQUAL_STRING("192.168.1.4", event.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("v-134297.local", event.hostname.c_str());
    TEST_ASSERT_EQUAL_STRING("134297", event.deviceId.c_str());
}

void test_display_message_defaults() {
    const Event event = makeDisplayMessageEvent("Test");
    TEST_ASSERT_EQUAL(EventType::DisplayMessageRequested, event.type);
    TEST_ASSERT_TRUE(event.scrollEnabled);
    TEST_ASSERT_EQUAL(Device::kDefaultScrollDelayMs, event.scrollDelayMs);
}

void test_display_message_custom_scroll() {
    const Event event = makeDisplayMessageEvent("Test", false, 500);
    TEST_ASSERT_FALSE(event.scrollEnabled);
    TEST_ASSERT_EQUAL(500, event.scrollDelayMs);
}

void test_network_disconnected_event() {
    const Event event = makeNetworkDisconnectedEvent();
    TEST_ASSERT_EQUAL(EventType::NetworkDisconnected, event.type);
}

void test_device_constants_lcd_geometry() {
    TEST_ASSERT_EQUAL(16, Device::kLcdCols);
    TEST_ASSERT_EQUAL(2, Device::kLcdRows);
    TEST_ASSERT_EQUAL(64, Device::kMaxMessageLen);
}

void test_device_constants_mqtt_defaults() {
    TEST_ASSERT_EQUAL(1883, Device::kMqttDefaultPort);
    TEST_ASSERT_EQUAL(512, Device::kMqttBufferSize);
}

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_display_status_event_fields);
    RUN_TEST(test_network_connected_event_fields);
    RUN_TEST(test_display_message_defaults);
    RUN_TEST(test_display_message_custom_scroll);
    RUN_TEST(test_network_disconnected_event);
    RUN_TEST(test_device_constants_lcd_geometry);
    RUN_TEST(test_device_constants_mqtt_defaults);
    return UNITY_END();
}
