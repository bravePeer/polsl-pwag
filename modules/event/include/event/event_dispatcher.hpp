#pragma once
#include "event/event.hpp"

class EventDispatcher
{
public:
    EventDispatcher(const Event& event)
    : event(event)
    { }

    template<typename T, typename F>
    bool dispatch(const F& fun)
    {
        if(event.getType() == T::GetStaticType())
        {
            fun(static_cast<const T&>(event));
            return true;
        }
        return false;
    }

private:
    const Event& event;
};