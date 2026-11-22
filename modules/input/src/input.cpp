#include "input/input.hpp"
#include "input/input_enum.hpp"

void Input::onKey(InputButtonId buttonId, InputAction action)
{
    if(action == InputAction::Press)
    {
        setButtonState(buttonId, Input::ButtonState::Press);
        publishKeyPressEvent(buttonId);
    }
    else if(action == InputAction::Release)
    {
        setButtonState(buttonId, Input::ButtonState::Release);
        publishKeyReleaseEvent(buttonId);
    }
}

void Input::onMouseCursor(double xpos, double ypos)
{
    mouse.lastPosition.x = mouse.position.x;
    mouse.lastPosition.y = mouse.position.y;

    mouse.position.x = xpos;
    mouse.position.y = ypos;

    mouse.diffPositions.x = mouse.lastPosition.x - xpos;
    mouse.diffPositions.y = mouse.lastPosition.y - ypos;

    publishMousePos(mouse.position.x, mouse.position.y);
}
