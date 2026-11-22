#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include "event/event.hpp"
#include "event/event_listener.hpp"

/// @brief EventBus is singleton
class EventBus
{
public:
    static EventBus& getEventBus()
    {
        static EventBus eventBus;
        return eventBus;
    }

    void setImmediateMode(bool immediate)
    {
        immediateMode = immediate;
    }

    void addListener(EventListener* listener);

    void removeListener(EventListener* listener);

    void publishEvent(const Event& event);

    void processEvent();

private:
    EventBus() = default;

    EventBus(const EventBus&)
    { }

    std::vector<EventListener*> listeners;
    std::queue<std::unique_ptr<Event>> eventQueue;
    std::mutex queueMutex;
    bool immediateMode = true;
};