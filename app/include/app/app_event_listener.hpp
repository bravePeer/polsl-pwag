#pragma once
#include "event/event_bus.hpp"
#include "event/event_listener.hpp"
#include "event/event_dispatcher.hpp"
#include "event/events/events_keyboard.hpp"
#include "input/input_enum.hpp"

class AppClose : public EventListener
{
public:
    explicit AppClose(bool& shouldExit)
    : shouldExit(shouldExit)
    {
        EventBus::getEventBus().addListener(this);
    }

    ~AppClose() override
    {
        EventBus::getEventBus().removeListener(this);
    }

    AppClose(const AppClose&) = delete;
    
    AppClose& operator=(const AppClose&) = delete;

    void onEvent(const Event& event) override
    {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<EventKeyPress>([this](const EventKeyPress& event){
            if(event.getKeyValue() == InputButtonId::Esc)
            {
                shouldExit = true;
            }
        });
    }
private:
    bool& shouldExit;
};