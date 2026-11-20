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
    KeyPress,
    KeyRelease,
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

class EventKeyPress : public Event
{
public:
    explicit EventKeyPress(int32_t keyCode)
    : keyCode(keyCode)
    { }

    static EventType getStaticType()
    {
        return EventType::KeyPress;
    }

    EventType getType() const override
    {
        return getStaticType();
    }

    Event* clone() const override
    {
        return new EventKeyPress(*this);
    }

    int32_t getKeyValue() const
    {
        return keyCode;
    }

private:
    /// @todo Temporary has type int not enum
    int32_t keyCode; 
};

class EventKeyRelease : public Event
{
public:
    explicit EventKeyRelease(int32_t keyCode)
    : keyCode(keyCode)
    { }

    static EventType getStaticType()
    {
        return EventType::KeyRelease;
    }

    EventType getType() const override
    {
        return getStaticType();
    }

    Event* clone() const override
    {
        return new EventKeyRelease(*this);
    }

    int32_t getKeyValue() const
    {
        return keyCode;
    }

private:
    /// @todo Temporary has type int not enum
    int32_t keyCode; 
};

class EventMousePos : public Event
{
public:
    explicit EventMousePos(double mousePosX, double mousePosY)
    : mousePosX(mousePosX), mousePosY(mousePosY)
    { }

    static EventType getStaticType()
    {
        return EventType::InputMousePos;
    }

    EventType getType() const override
    {
        return getStaticType();
    }

    Event* clone() const override
    {
        return new EventMousePos(*this);
    }

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
