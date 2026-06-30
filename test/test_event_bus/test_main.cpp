#include <unity.h>
#include "core/EventBus.h"
#include "core/Events.h"

void setUp() {
    // fresh bus per test via stack allocation in each test
}

void tearDown() {}

void test_publish_delivers_to_subscriber() {
    EventBus bus;
    int callCount = 0;

    bus.subscribe(EventType::DisplayMessageRequested, [&](const Event& event) {
        callCount++;
        TEST_ASSERT_EQUAL(EventType::DisplayMessageRequested, event.type);
        TEST_ASSERT_EQUAL_STRING("Hello", event.message.c_str());
    });

    bus.publish(makeDisplayMessageEvent("Hello", false, 200));
    TEST_ASSERT_EQUAL(1, callCount);
}

void test_publish_only_matching_type() {
    EventBus bus;
    int displayCalls = 0;
    int networkCalls = 0;

    bus.subscribe(EventType::DisplayMessageRequested,
                  [&](const Event&) { displayCalls++; });
    bus.subscribe(EventType::NetworkConnected, [&](const Event&) { networkCalls++; });

    bus.publish(makeDisplayMessageEvent("A"));
    bus.publish(makeNetworkConnectedEvent("1.2.3.4", "v-1.local", "1"));

    TEST_ASSERT_EQUAL(1, displayCalls);
    TEST_ASSERT_EQUAL(1, networkCalls);
}

void test_null_handler_rejected() {
    EventBus bus;
    bus.subscribe(EventType::DisplayRestoreNetwork, std::function<void(const Event&)>());
    TEST_ASSERT_EQUAL(0, bus.handlerCount());
}

void test_handler_limit_enforced() {
    EventBus bus;
    for (size_t i = 0; i < EventBus::kMaxHandlers + 2; i++) {
        bus.subscribe(EventType::DisplayStatus, [](const Event&) {});
    }
    TEST_ASSERT_EQUAL(EventBus::kMaxHandlers, bus.handlerCount());
}

void test_restore_network_event_type() {
    EventBus bus;
    EventType received = EventType::DisplayMessageRequested;

    bus.subscribe(EventType::DisplayRestoreNetwork,
                  [&](const Event& event) { received = event.type; });

    bus.publish(makeDisplayRestoreNetworkEvent());
    TEST_ASSERT_EQUAL(EventType::DisplayRestoreNetwork, received);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_publish_delivers_to_subscriber);
    RUN_TEST(test_publish_only_matching_type);
    RUN_TEST(test_null_handler_rejected);
    RUN_TEST(test_handler_limit_enforced);
    RUN_TEST(test_restore_network_event_type);
    return UNITY_END();
}
