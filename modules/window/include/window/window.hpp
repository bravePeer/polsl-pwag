#pragma once
#include <GLFW/glfw3.h>
#include "window/window_data_provider.hpp"
#include "event/event_bus.hpp"

class Window
{
public:
    Window(int width, int height, const char *title, Input& input, EventBus& eventBus = EventBus::getEventBus())
    : input(input), eventBus(eventBus)
    {
        if(glfwInit() != GLFW_TRUE)
        {
            throw std::runtime_error("GLFW init error");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if(window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation error");
        }
    }
    ~Window()
    {
        glfwSetCursorPos(window, storedCursorX, storedCursorY);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    GLFWwindow* getWindow() const
    {
        return window;
    }

    void setDataProvider(const WindowDataProvider& dataProvider)
    {
        windowDataProvider = dataProvider;
    }
    
    /// @brief Use events
    void initWindowCallbacks();

    void pollEvents() const;

    bool shouldClose() const;

    /// @brief Checks frame buffer size and waits if is 0x0
    /// @return true if frame buffer was 0x0 (minimized)
    bool waitIfMinimize();

    std::vector<const char*> getRequiredInstanceExtensions() const;
    
private:
    GLFWwindow* window = nullptr;
    Input& input;
    EventBus& eventBus;
    WindowDataProvider windowDataProvider;

    double storedCursorX = 0.;
    double storedCursorY = 0.;
};