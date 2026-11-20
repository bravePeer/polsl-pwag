#pragma once
#include "event/event.hpp"

class EventListener
{
public:
    virtual ~EventListener() = default;
    virtual void onEvent(const Event& event) = 0;
};