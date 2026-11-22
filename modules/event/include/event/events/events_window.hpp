#pragma once
#include "event/event.hpp"
#include <cstdint>

class EventWindowResize : public EventBase<EventWindowResize, EventType::WindowResize>
{
public:
    EventWindowResize(int32_t width, int32_t height)
    : windowWidth(width), windowHeight(height)
    { }

    int32_t getWindowWidth() const
    {
        return windowWidth;
    }

    int32_t getWindowHeight() const
    {
        return windowHeight;
    }

private:
    int32_t windowWidth;
    int32_t windowHeight;
};
