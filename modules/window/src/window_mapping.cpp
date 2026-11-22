#include "window/window_mapping.hpp"
#include <GLFW/glfw3.h>

std::optional<InputButtonId> glfwKeyToButtonId(int glfwKey)
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

std::optional<InputButtonId> glfwMouseButtonToButtonId(int glfwButton)
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

InputAction glfwActionToInputAction(int action)
{
    using enum InputAction;
    switch(action)
    {
    case GLFW_RELEASE:
        return Release;
    case GLFW_PRESS:
        return Press;
    case GLFW_REPEAT:
        return Repeat;
    default:
        break;
    }
    return Release;
}
