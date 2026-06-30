#pragma once

#include <Arduino.h>
#include "core/DeviceConstants.h"

enum class EventType {
    NetworkConnected,
    NetworkDisconnected,
    DisplayStatus,
    DisplayMessageRequested,
    DisplayRestoreNetwork
};

struct Event {
    EventType type;

    String line1;
    String line2;
    String ip;
    String hostname;
    String deviceId;
    String message;
    bool scrollEnabled = true;
    uint16_t scrollDelayMs = Device::kDefaultScrollDelayMs;
};

inline Event makeDisplayStatusEvent(const String& line1, const String& line2 = "") {
    Event event{};
    event.type = EventType::DisplayStatus;
    event.line1 = line1;
    event.line2 = line2;
    return event;
}

inline Event makeNetworkConnectedEvent(const String& ip, const String& hostname, const String& deviceId) {
    Event event{};
    event.type = EventType::NetworkConnected;
    event.ip = ip;
    event.hostname = hostname;
    event.deviceId = deviceId;
    return event;
}

inline Event makeNetworkDisconnectedEvent() {
    Event event{};
    event.type = EventType::NetworkDisconnected;
    return event;
}

inline Event makeDisplayMessageEvent(const String& message,
                                     bool scrollEnabled = true,
                                     uint16_t scrollDelayMs = Device::kDefaultScrollDelayMs) {
    Event event{};
    event.type = EventType::DisplayMessageRequested;
    event.message = message;
    event.scrollEnabled = scrollEnabled;
    event.scrollDelayMs = scrollDelayMs;
    return event;
}

inline Event makeDisplayRestoreNetworkEvent() {
    Event event{};
    event.type = EventType::DisplayRestoreNetwork;
    return event;
}
