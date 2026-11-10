#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(int width, int height, const char *title)
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
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    GLFWwindow* getWindow() const
    {
        return window;
    }

    void setWindowResizeCb(GLFWframebuffersizefun callback, void* userPointer)
    {
        glfwSetWindowUserPointer(window, userPointer);
        glfwSetFramebufferSizeCallback(window, callback);
    }

    void pollEvents()
    {
        glfwPollEvents();
    }

    bool shouldClose() const
    {
        return glfwWindowShouldClose(window);
    }

    /// @brief Checks frame buffer size and waits if is 0x0
    /// @return true if frame buffer was 0x0 (minimized)
    bool waitIfMinimize()
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

    std::vector<const char*> getRequiredInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }
    
private:
    GLFWwindow* window = nullptr;
};