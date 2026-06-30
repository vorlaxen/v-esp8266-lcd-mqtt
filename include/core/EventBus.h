#pragma once

#include "core/Events.h"
#include <functional>

class EventBus {
public:
    static constexpr size_t kMaxHandlers = 16;

    void subscribe(EventType type, std::function<void(const Event&)> handler);
    void publish(const Event& event);

    size_t handlerCount() const { return handlerCount_; }

private:
    struct HandlerEntry {
        EventType type;
        std::function<void(const Event&)> handler;
    };

    HandlerEntry handlers_[kMaxHandlers];
    size_t handlerCount_ = 0;
};
