#pragma once
#include <iostream>
#include <stdexcept>
#include <chrono>

#include "app/app_config.hpp"
#include "app/window.hpp"
#include "vulkan_engine/vulkan_engine.hpp"
#include "vulkan_engine/vulkan_resource_manager.hpp"

#include "vertex/object.hpp"

class App
{
public:
    App(int width, int height, const char* title)
    : window(width, height, title),
      windowProvider{
        .instanceExtensions = window.getRequiredInstanceExtensions(),
        .getFramebufferSize = [this]{
          int w, h;
          glfwGetFramebufferSize(window.getWindow(), &w, &h);
          return std::pair{w, h};
      },
      .createSurface = [this](VkInstance instance){
          VkSurfaceKHR s;
          if (glfwCreateWindowSurface(instance, window.getWindow(), nullptr, &s) != VK_SUCCESS)
              throw std::runtime_error("surface");
          return s;
      }},
      vkEngine(windowProvider)
    {
        window.setWindowResizeCb([](GLFWwindow* window, int width, int height){
            auto engine = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            engine->setWindowResized(true);
        }, &vkEngine);

        objects.push_back(createRectangle());
        objects.push_back(createRectangle());
        objects.push_back(createRectangle());

        objects[1].transform.move({0.5f, 0.f, 0.f});
        objects[2].transform.move({-0.5f, 0.5f, 0.f});
        objects[1].transform.scale.x = 0.5f;
        objects[1].transform.scale.y = 0.5f;
        objects[1].transform.scale.z = 0.5f;

        vkEngine.getResourceManager().loadObject(objects[1].mesh);
        vkEngine.getResourceManager().getVkObject().setModelTransform(objects[1].transform.getModelMatrix());
    }
    
    App(const App&) = delete;

    ~App() noexcept
    {

    }

    void run()
    {
        while (!window.shouldClose())
        {
            updateTime();

            // Process inputs
            window.pollEvents();

            // Updates
            fixedUpdate();
            update();

            // Draw frame
            if(vkEngine.drawFramePub())
            { /// Recreating swap chan
                window.waitIfMinimize();
                if(window.shouldClose())
                {
                    break;
                }
                vkEngine.recreateSwapChainPub();
            }
        }
    }

private:
    Window window;
    WindowProvider windowProvider;
    VulkanEngine vkEngine;

    std::vector<Object> objects;

    std::chrono::time_point<std::chrono::steady_clock> frameLastTime;
    std::chrono::duration<float> deltaTime;
    void updateTime()
    {
        auto now = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(now - frameLastTime);
        frameLastTime = now;
    }

    float getFps()
    {
        return 1.f / deltaTime.count();
    }

    void fixedUpdate()
    {

    }

    void update()
    {

    }
};