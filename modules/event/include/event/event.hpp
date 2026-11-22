#pragma once
#include <memory>

enum class EventCategory
{
    None = 0,
    Application = 1 << 0,
    Window      = 1 << 1,
    Input       = 1 << 2,
    Keyboard    = 1 << 3,
    Mouse       = 1 << 4,
    MouseButton = 1 << 5,
};

enum class EventPriority
{

};

enum class EventType
{
    WindowResize,

    InputKey,
    MouseKey,
    MousePos,
    KeyPress,
    KeyRelease,
};

class Event
{
public:
    virtual ~Event() = default;

    virtual EventType getType() const = 0;

    virtual std::unique_ptr<Event> clone() const = 0;
};

/// @brief 
/// @tparam D Derived
/// @tparam T Type of event
template <typename D, EventType T>
class EventBase : public Event
{
public:
    static constexpr EventType getStaticType()
    {
        return T;
    }

    EventType getType() const override
    {
        return T;
    }
    
    std::unique_ptr<Event> clone() const override
    {
        return std::make_unique<D>(static_cast<const D&>(*this));
    }
};
