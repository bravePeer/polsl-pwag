#pragma once
#include "event/event.hpp"

class EventMousePos : public EventBase<EventMousePos, EventType::MousePos>
{
public:
    explicit EventMousePos(double mousePosX, double mousePosY)
    : mousePosX(mousePosX), mousePosY(mousePosY)
    { }

    double getMousePosX() const
    {
        return mousePosX;
    }

    double getMousePosY() const
    {
        return mousePosY;
    }

private:
    double mousePosX;
    double mousePosY;
};