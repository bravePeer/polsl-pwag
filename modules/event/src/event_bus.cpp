#include "event/event.hpp"
#include <algorithm>

#include "event/event_bus.hpp"

void EventBus::addListener(EventListener* listener)
{
    listeners.push_back(listener);
}

void EventBus::removeListener(EventListener* listener)
{
    auto it = std::ranges::find(listeners, listener);
    if(it != listeners.end())
    {
        listeners.erase(it);
    }
}

void EventBus::publishEvent(const Event& event)
{
    if(immediateMode)
    {
        for(auto listener : listeners)
        {
            listener->onEvent(event);
        }
    }
    else
    {
        std::scoped_lock lock{queueMutex};
        eventQueue.push(std::unique_ptr<Event>(event.clone()));
    }
}

void EventBus::processEvent()
{
    if(immediateMode)
    {
        return;
    }

    std::queue<std::unique_ptr<Event>> currentEvents;

    {
        std::scoped_lock lock{queueMutex};
        std::swap(currentEvents, eventQueue);
    }

    while(!currentEvents.empty())
    {
        const auto& event = *currentEvents.front();
        for(auto listener : listeners)
        {
            listener->onEvent(event);
        }
        currentEvents.pop();
    }
}
