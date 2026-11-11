#pragma once
#include <array>
#include <utility>
#include <optional>

/// State flow:
/// RRRRR -> PPP -> C -> RRR

class Input
{
public:
    enum class ButtonId : unsigned int
    {
        // Keyboard
        W,
        A,
        S,
        D,
        Esc,

        // Mouse
        MouseLeft,
        MouseRight,
        MouseMiddle,

        Count
    };

    enum class ButtonState
    {
        Release,
        Press,
        Clicked
    };

    bool isButtonPressed(ButtonId buttonId)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(ButtonId::Count))
        {
            /// @todo Log error
            return false;
        }
        return buttons[std::to_underlying(buttonId)] == ButtonState::Press;
    }

    bool isButtonReleased(ButtonId buttonId)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(ButtonId::Count))
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
    
    std::optional<Input::ButtonId> mouseButtonToButtonId(int glfwButton);

    std::optional<Input::ButtonId> keyToButtonId(int glfwKey);

    void onKey(int glfwKey, int glfwAction);

    void onMouseButton(int glfwButton, int glfwAction);

    void onMouseCursor(double xpos, double ypos);

private:
    std::array<ButtonState, std::to_underlying(ButtonId::Count)> buttons;

    Mouse mouse;

    void setButtonState(ButtonId buttonId, ButtonState state)
    {
        if(std::to_underlying(buttonId) >= std::to_underlying(ButtonId::Count))
        {
            /// @todo Log error
            return;
        }
        buttons[std::to_underlying(buttonId)] = state;
    }
};
