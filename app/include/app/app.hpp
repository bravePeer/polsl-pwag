#pragma once
#include <iostream>
#include <stdexcept>
#include <chrono>

#include "app/app_config.hpp"
#include "app/window.hpp"
#include "vulkan_engine/vulkan_engine.hpp"
#include "vulkan_engine/vulkan_resource_manager.hpp"

#include "vertex/object.hpp"

#include "component/component.hpp"
#include "component/component_spamer.hpp"
#include "component/component_transform.hpp"
#include "component/component_camera.hpp"
#include "component/entity.hpp"
#include "component/component_utils.hpp"

#include "resource/resource.hpp"
#include "resource/resource_manager.hpp"

#include <memory>

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
      vkEngine(windowProvider),
      resourceManager(vkEngine.getContext())
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

        // resourceManager.load<ResourceMesh>("test");
        ResourceMesh::ResourceMeshData data{
            .vertices = triangleVertices,
            .indices = triangleIndices
        };
        resourceManager.load<ResourceMesh>("test2", std::make_unique<ResourceMesh::ResourceMeshData>(data));

        resourceManager.getResource<ResourceMesh>("test2");

        Entity spammer;
        spammer.addComponent<ComponentSpamer>(1, "Spam");
        entities.push_back(std::move(spammer));
        Entity spammer2;
        spammer2.addComponent<ComponentSpamer>(2, "Spam2");
        entities.push_back(std::move(spammer2));

        componentsDump(entities);
    }
    
    App(const App&) = delete;

    ~App() noexcept
    {

    }

    void run()
    {
        frameLastTime = std::chrono::steady_clock::now();
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
    ResourceManager resourceManager;

    std::vector<Object> objects;
    std::vector<Entity> entities;

    std::chrono::time_point<std::chrono::steady_clock> frameLastTime;
    std::chrono::duration<float> deltaTimeDuration;
    void updateTime()
    {
        auto now = std::chrono::steady_clock::now();
        deltaTimeDuration = std::chrono::duration<float>(now - frameLastTime);
        frameLastTime = now;
    }

    float getFps()
    {
        return 1.f / deltaTimeDuration.count();
    }

    void fixedUpdate()
    {

    }

    void update()
    {
        const float deltaTime = deltaTimeDuration.count();

        for(auto& entity : entities)
        {
            entity.update(deltaTime);
        }
    }
};