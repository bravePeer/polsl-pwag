#pragma once
#include "event/event.hpp"
#include "input/input_enum.hpp"

template <typename D, EventType T>
class EventKey : public EventBase<D, T>
{
public:
    explicit EventKey(InputButtonId keyCode)
    : keyCode(keyCode)
    { }

    InputButtonId getKeyValue() const
    {
        return keyCode;
    }

private:
    InputButtonId keyCode; 
};
