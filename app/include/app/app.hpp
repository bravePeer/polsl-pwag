#pragma once
#include "window/window.hpp"
#include "engine/engine.hpp"

#include "component/entity.hpp"
#include "component/component_camera.hpp"
#include "component/scene.hpp"

#include "resource/resource_manager.hpp"
#include "app/app_event_listener.hpp"

class App
{
public:
    App(int width, int height, const char* title)
    : window(width, height, title, input),
      vkEngine(windowProvider),
      appClose(shouldExit)
    {
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
            return window.getFrameBufferSize();
      },
      .createSurface = [this](VkInstance instance){
        return window.createSurface(instance);
      }
    };
    VulkanEngine vkEngine;
    ResourceManager resourceManager;
    SceneCulling culling;

    Scene scene;

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

        vkEngine.setCamera(camera->getViewMatrix(), camera->getProjectionMatrix());
    }

    void render()
    {
        std::vector<Entity*> visibleEntities = culling.getVisibleEntities(scene);
        vkEngine.addEntitiesToDraw(visibleEntities);
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
