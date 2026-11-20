#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include "event/event.hpp"
#include "event/event_listener.hpp"

class EventBus
{
public:

    void setImmediateMode(bool immediate)
    {
        immediateMode = immediate;
    }

    void addListener(EventListener* listener)
    {
        listeners.push_back(listener);
    }

    void removeListener(EventListener* listener)
    {
        auto it = std::ranges::find(listeners, listener);
        if(it != listeners.end())
        {
            listeners.erase(it);
        }
    }

    void publishEvent(const Event& event)
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

    void processEvent()
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
            auto& event = *currentEvents.front();
            for(auto listener : listeners)
            {
                listener->onEvent(event);
            }
            currentEvents.pop();
        }
    }

private:
    std::vector<EventListener*> listeners;
    std::queue<std::unique_ptr<Event>> eventQueue;
    std::mutex queueMutex;
    bool immediateMode = true;

};