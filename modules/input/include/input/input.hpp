#pragma once
#include <array>
#include <utility>
#include <optional>
#include "input/input_enum.hpp"
#include <type_traits>
#include "event/event_bus.hpp"
#include "event/event.hpp"

/// State flow:
/// RRRRR -> PPP -> C -> RRR

class Input
{
public:
    enum class ButtonState
    {
        Release,
        Press,
        Clicked
    };

    bool isButtonPressed(InputButtonId buttonId)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(InputButtonId::Count))
        {
            /// @todo Log error
            return false;
        }
        return buttons[std::to_underlying(buttonId)] == ButtonState::Press;
    }

    bool isButtonReleased(InputButtonId buttonId)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(InputButtonId::Count))
        {
            /// @todo Log error
            return false;
        }
        return buttons[std::to_underlying(buttonId)] == ButtonState::Release;
    }

    struct MousePosition
    {
        // is double ok or too big?
        double x;
        double y;
    };

    struct Mouse
    {
        MousePosition position;
        MousePosition lastPosition;
        MousePosition diffPositions;
    };

    Mouse& getMouse()
    {
        return mouse;
    }
    
    std::optional<InputButtonId> mouseButtonToButtonId(int glfwButton);

    std::optional<InputButtonId> keyToButtonId(int glfwKey);

    void onKey(int glfwKey, int glfwAction);

    void onMouseButton(int glfwButton, int glfwAction);

    void onMouseCursor(double xpos, double ypos);

private:
    EventBus& eventBus = EventBus::getEventBus();

    std::array<ButtonState, std::to_underlying(InputButtonId::Count)> buttons;

    Mouse mouse;

    void setButtonState(InputButtonId buttonId, ButtonState state)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(InputButtonId::Count))
        {
            /// @todo Log error
            return;
        }
        buttons[std::to_underlying(buttonId)] = state;
    }

    void publishKeyPressEvent(InputButtonId buttonId)
    {
        EventKeyPress event(static_cast<int>(std::to_underlying(buttonId)));
        eventBus.publishEvent(event);
    }

    void publishKeyReleaseEvent(InputButtonId buttonId)
    {
        EventKeyRelease event(static_cast<int>(std::to_underlying(buttonId)));
        eventBus.publishEvent(event);
    }

    void publishMousePos(double x, double y)
    {
        eventBus.publishEvent(EventMousePos(x, y));
    }
};
