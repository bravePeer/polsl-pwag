#include "window/window.hpp"
#include "event/events/events_window.hpp"
#include "window/window_mapping.hpp"

void Window::initWindowCallbacks()
{
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, 
        [](GLFWwindow* glfwWindow, int width, int height){
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            assert(win);
            EventWindowResize event(width, height);
            win->eventBus.publishEvent(event);
        }
    );
    glfwSetKeyCallback(window, 
        [](GLFWwindow* glfwWindow, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods){
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            if(auto btn = glfwKeyToButtonId(key))
            {
                win->input.onKey(*btn, glfwActionToInputAction(action));
            }
        }
    );
    glfwSetMouseButtonCallback(window, 
        [](GLFWwindow* glfwWindow, int button, int action, [[maybe_unused]] int mods){
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            if(auto btn = glfwMouseButtonToButtonId(button))
            {
                win->input.onKey(*btn, glfwActionToInputAction(action));
            }
        }
    );

    glfwGetCursorPos(window, &storedCursorX, &storedCursorY);
    glfwSetCursorPos(window, 0., 0.);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, 
        [](GLFWwindow* glfwWindow, double xpos, double ypos){
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            win->input.onMouseCursor(xpos, ypos);
        }
    );
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

/// @brief Checks frame buffer size and waits if is 0x0
/// @return true if frame buffer was 0x0 (minimized)
bool Window::waitIfMinimize()
{
    int width = 0;
    int height = 0;
    
    glfwGetFramebufferSize(window, &width, &height);
    if(width != 0 && height != 0)
    {
        return false;
    }

    while ((width == 0 || height == 0) && !glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    return true;
}

std::vector<const char*> Window::getRequiredInstanceExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}
