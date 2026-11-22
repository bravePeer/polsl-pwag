#pragma once
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <memory>

#include "app/app_config.hpp"
#include "window/window.hpp"
#include "engine/engine.hpp"
#include "engine/buffer_creator.hpp"

#include "component/entity.hpp"
#include "component/component_spamer.hpp"
#include "component/component_transform.hpp"
#include "component/component_camera.hpp"
#include "component/component_utils.hpp"
#include "component/component_mesh.hpp"
#include "component/component_any.hpp"

#include "resource/resource_manager.hpp"
#include "resource/resource_mesh.hpp"
#include "resource/resource_models.hpp"

#include "component/scene.hpp"

#include "window/window_data_provider.hpp"
#include "component/component_movement.hpp"

#include "app/app_event_listener.hpp"

class App
{
public:
    App(int width, int height, const char* title)
    : window(width, height, title, input),
    //   windowProvider,
      vkEngine(windowProvider),
      resourceManager(vkEngine.getContext()),
      buffCreator(vkEngine.getContext()),
      renderer(vkEngine.getContext()),
      appClose(shouldExit)
    {
        vkEngine.setBufferCreator(&buffCreator);

        windowDataProvider = {
            .engine = &vkEngine,
            .input = &input
        };

        window.setDataProvider(windowDataProvider);
        window.initWindowCallbacks();

        vkEngine.setResourceManager(&resourceManager);
    }
    
    App(const App&) = delete;

    ~App() noexcept
    { 
        vkEngine.getContext().device.waitIdle();
    }

    void init();

    void run()
    {
        frameLastTime = std::chrono::steady_clock::now();
        while (!window.shouldClose() && !shouldExit)
        {
            if(window.waitIfMinimize())
            {
                frameLastTime = std::chrono::steady_clock::now();
                continue;
            }

            updateTime();

            // Process inputs
            window.pollEvents();

            // Updates
            fixedUpdate();

            update();

            render();
        }
    }

    void exit()
    {
        shouldExit = true;
    }

private:
    bool shouldExit = false;

    Input input;
    Window window;
    WindowProvider windowProvider={
        .instanceExtensions = window.getRequiredInstanceExtensions(),
        .getFramebufferSize = [this](){
          int w;
          int h;
          glfwGetFramebufferSize(window.getWindow(), &w, &h);
          return std::pair{w, h};
      },
      .createSurface = [this](VkInstance instance){
          VkSurfaceKHR s;
          if (glfwCreateWindowSurface(instance, window.getWindow(), nullptr, &s) != VK_SUCCESS)
              throw std::runtime_error("surface");
          return s;
      }
    };
    VulkanEngine vkEngine;
    ResourceManager resourceManager;
    SceneCulling culling;
    BufferCreator buffCreator;
    Renderer renderer;

    Scene scene;

    WindowDataProvider windowDataProvider;

    AppClose appClose;

    std::chrono::time_point<std::chrono::steady_clock> frameLastTime;
    std::chrono::duration<float> deltaTimeDuration;
    void updateTime()
    {
        auto now = std::chrono::steady_clock::now();
        deltaTimeDuration = std::chrono::duration<float>(now - frameLastTime);
        frameLastTime = now;
    }

    float getFps() const
    {
        return 1.f / deltaTimeDuration.count();
    }

    void fixedUpdate()
    {
        /// @todo Add fixed update
    }

    void update()
    {
        const float deltaTime = deltaTimeDuration.count();
        scene.update(deltaTime);
        
        auto camera = scene.getEntityByName("main_camera")->getComponent<ComponentCamera>();

        buffCreator.set(camera->getViewMatrix(), camera->getProjectionMatrix());
    }

    void render()
    {
        std::vector<Entity*> visibleEntities = culling.getVisibleEntities(scene);
        renderer.addEntities(visibleEntities);
        vkEngine.drawFrame();
    }

    void initMainCamera()
    {
        Entity* entityWithCamera = scene.getEntityByName("main_camera");
        if(!entityWithCamera)
        {
            throw std::runtime_error("No camera on scene");
        }
        culling.setCamera(entityWithCamera->getComponent<ComponentCamera>());
    }
};
