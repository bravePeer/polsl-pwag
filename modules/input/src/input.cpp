#include "input/input.hpp"
#include <GLFW/glfw3.h>
#include <optional>
#include "input/input_enum.hpp"

std::optional<InputButtonId> Input::keyToButtonId(int glfwKey)
{
    switch(glfwKey)
    {
    using enum InputButtonId;
    case GLFW_KEY_W: 
        return W;
    case GLFW_KEY_A: 
        return A;
    case GLFW_KEY_S: 
        return S;
    case GLFW_KEY_D: 
        return D;
    case GLFW_KEY_ESCAPE:
        return Esc;
    default:
        return std::nullopt;
    }
}

void Input::onKey(int glfwKey, int glfwAction)
{
    if(auto id = keyToButtonId(glfwKey))
    {
        if(glfwAction == GLFW_PRESS)
        {
            setButtonState(*id, Input::ButtonState::Press);
            publishKeyPressEvent(*id);
        }
        else if(glfwAction == GLFW_RELEASE)
        {
            setButtonState(*id, Input::ButtonState::Release);
            publishKeyReleaseEvent(*id);
        }
    }
}

std::optional<InputButtonId> Input::mouseButtonToButtonId(int glfwButton)
{
    switch(glfwButton)
    {
    using enum InputButtonId;
    case GLFW_MOUSE_BUTTON_LEFT: 
        return MouseLeft;
    case GLFW_MOUSE_BUTTON_RIGHT: 
        return MouseRight;
    case GLFW_MOUSE_BUTTON_MIDDLE: 
        return MouseMiddle;
    default:
        return std::nullopt;
    }
}

void Input::onMouseButton(int glfwButton, int glfwAction)
{
    if(auto id = mouseButtonToButtonId(glfwButton))
    {
        if(glfwAction == GLFW_PRESS)
        {
            setButtonState(*id, Input::ButtonState::Press);
            publishKeyPressEvent(*id);
        }
        else if(glfwAction == GLFW_RELEASE)
        {
            setButtonState(*id, Input::ButtonState::Release);
            publishKeyReleaseEvent(*id);
        }
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
