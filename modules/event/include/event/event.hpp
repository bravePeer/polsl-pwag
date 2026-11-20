#pragma once
#include <cstdint>

enum class EventCategory
{

};

enum class EventPriority
{

};

enum class EventType
{
    WindowResize,

    InputKey,
    InputMouseKey,
    InputMousePos,

};

class Event
{
public:
    virtual ~Event() = default;

    virtual EventType getType() const = 0;

    virtual Event* clone() const = 0;
};

class EventWindowResize : public Event
{
public:
    EventWindowResize(int32_t width, int32_t height)
    : windowWidth(width), windowHeight(height)
    { }

    static EventType getStaticType()
    {
        return EventType::WindowResize;
    }

    EventType getType() const override
    {
        return getStaticType();
    }

    Event* clone() const override
    {
        return new EventWindowResize(*this);
    }

    int32_t getWindowWidth()
    {
        return windowWidth;
    }

    int32_t getWindowHeight()
    {
        return windowHeight;
    }

private:
    int32_t windowWidth;
    int32_t windowHeight;
};

class EventInputKey : public Event
{
public:
    EventInputKey(int32_t keyCode, int32_t keyState)
    : keyCode(keyCode), keyState(keyState)
    { }

    static EventType getStaticType()
    {
        return EventType::InputKey;
    }

    EventType getType() const override
    {
        return getStaticType();
    }

    Event* clone() const override
    {
        return new EventInputKey(*this);
    }

    int32_t getKeyValue()
    {
        return keyCode;
    }

    int32_t getKeyState()
    {
        return keyState;
    }

private:
    /// @todo Temporary has type int not enum
    int32_t keyCode; 
    /// @todo Temporary has type int not enum
    int32_t keyState; 
};