#pragma once
#include "event/event.hpp"

class EventDispatcher
{
public:
    EventDispatcher(const Event& eventToDispatch)
    : eventToDispatch(eventToDispatch)
    { }

    template<typename T, typename F>
    bool dispatch(const F& fun)
    {
        if(eventToDispatch.getType() == T::getStaticType())
        {
            fun(static_cast<const T&>(eventToDispatch));
            return true;
        }
        return false;
    }

private:
    const Event& eventToDispatch;
};