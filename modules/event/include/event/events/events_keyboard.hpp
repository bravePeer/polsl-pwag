#pragma once
#include "event/events/events_key.hpp"

class EventKeyPress : public EventKey<EventKeyPress, EventType::KeyPress>
{
    using EventKey::EventKey;
};

class EventKeyRelease : public EventKey<EventKeyRelease, EventType::KeyRelease>
{
    using EventKey::EventKey;
};
