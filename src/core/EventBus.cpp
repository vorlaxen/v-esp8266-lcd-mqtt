#include "core/EventBus.h"
#include "core/Log.h"

void EventBus::subscribe(EventType type, std::function<void(const Event&)> handler) {
    if (!handler) {
        Log::warn("EventBus: null handler rejected");
        return;
    }

    if (handlerCount_ >= kMaxHandlers) {
        Log::error("EventBus: handler limit reached");
        return;
    }

    handlers_[handlerCount_].type = type;
    handlers_[handlerCount_].handler = handler;
    handlerCount_++;
}

void EventBus::publish(const Event& event) {
    for (size_t i = 0; i < handlerCount_; i++) {
        if (handlers_[i].type == event.type && handlers_[i].handler) {
            handlers_[i].handler(event);
        }
    }
}
