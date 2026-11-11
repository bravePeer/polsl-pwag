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
      resourceManager(vkEngine.getContext()),
      buffCreator(vkEngine.getContext()),
      renderer(vkEngine.getContext())
    {
        vkEngine.setBufferCreator(&buffCreator);

        windowFunProvider = {
            .frameBufferSizeFun = [](GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height){
                auto data = static_cast<WindowDataProvider*>(glfwGetWindowUserPointer(window));
                if(data && data->engine)
                {
                    data->engine->setWindowResized(true);
                }
            },
            .keyFun = [](GLFWwindow* window, int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods){
                auto data = static_cast<WindowDataProvider*>(glfwGetWindowUserPointer(window));
                if(data && data->input)
                {
                    data->input->onKey(key, action);
                }
            },
            .mouseButtonFun = [](GLFWwindow* window, int button, int action, [[maybe_unused]] int mods){
                auto data = static_cast<WindowDataProvider*>(glfwGetWindowUserPointer(window));
                if(data && data->input)
                {
                    data->input->onMouseButton(button, action);
                }
            },
            .mouseCursorFun = [](GLFWwindow* window, double xpos, double ypos){
                auto data = static_cast<WindowDataProvider*>(glfwGetWindowUserPointer(window));
                if(data && data->input)
                {
                    data->input->onMouseCursor(xpos, ypos);
                }
            }
        };

        windowDataProvider = {
            .engine = &vkEngine,
            .input = &input
        };

        window.initWindowCallbacks(&windowFunProvider, &windowDataProvider);

        vkEngine.setResourceManager(&resourceManager);
    }
    
    App(const App&) = delete;

    ~App() noexcept
    { 
        vkEngine.getContext().device.waitIdle();
    }

    void init()
    {
        resourceManager.load<ResourceMesh>("cube", 
            std::make_unique<ResourceMesh::ResourceMeshData>(
                createCube(.5f,.5f,.5f)
        ));
        auto cubeMesh = resourceManager.getResource<ResourceMesh>("cube");
        resourceManager.load<ResourceMesh>("cone", 
            std::make_unique<ResourceMesh::ResourceMeshData>(
                createCone(.3f, .4f)
        ));
        auto coneMesh = resourceManager.getResource<ResourceMesh>("cone");
        resourceManager.loadToGpu();

        auto& camera = scene.createEntity("main_camera");
        camera.addComponent<ComponentCamera>();
        camera.addComponent<ComponentTransform>();

        auto& mesh1 = scene.createEntity();
        mesh1.addComponent<ComponentMesh>()->setMesh(cubeMesh);
        mesh1.addComponent<ComponentTransform>()->setPosition({0.f, -0.5f, -5.5f});
        mesh1.addComponent<ComponentAny>([](ComponentAny* comp, float deltaTime){
            auto transform = comp->getOwner()->getComponent<ComponentTransform>();
            const float angle = 3.141f * deltaTime / 2;
            transform->rotate(angle, glm::vec3(0.f, 1.f, 1.f));
        });

        auto& mesh2 = scene.createEntity();
        mesh2.addComponent<ComponentMesh>()->setMesh(cubeMesh);
        mesh2.addComponent<ComponentTransform>()->setPosition({0.f, 0.f, -10.5f});
        mesh2.addComponent<ComponentAny>([](ComponentAny* comp, float deltaTime){
            auto transform = comp->getOwner()->getComponent<ComponentTransform>();
            const float angle = 3.141f * deltaTime / 2;
            transform->rotate(angle, glm::vec3(0.f, 1.f, 0.f));
        });

        auto& mesh3 = scene.createEntity();
        mesh3.addComponent<ComponentMesh>()->setMesh(coneMesh);
        mesh3.addComponent<ComponentTransform>()->setPosition({-.4f, 0.f, -10.5f});
        mesh3.addComponent<ComponentAny>([](ComponentAny* comp, float deltaTime){
            auto transform = comp->getOwner()->getComponent<ComponentTransform>();
            const float angle = 3.141f * deltaTime / 2;
            transform->rotate(angle, glm::vec3(1.f, 1.f, 1.f));
        });

        buffCreator.addToGenerate(scene.getEntities());
        buffCreator.createBuffers();
        buffCreator.createDescriptorSet();
        buffCreator.bindUboOffsets();
        vkEngine.init();
        vkEngine.setRenderer(&renderer);
        
        renderer.setBuffCreator(&buffCreator);
        renderer.setResourceManager(&resourceManager);
    }

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

private:
    bool shouldExit = false;

    Window window;
    WindowProvider windowProvider;
    VulkanEngine vkEngine;
    ResourceManager resourceManager;
    SceneCulling culling;
    BufferCreator buffCreator;
    Renderer renderer;
    Input input;

    Scene scene;

    WindowFunProvider windowFunProvider;
    WindowDataProvider windowDataProvider;

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
        scene.update(deltaTime);

        if(input.isButtonPressed(Input::ButtonId::Esc))
        {
            shouldExit = true;
            return;
        }

        if(input.isButtonPressed(Input::ButtonId::W))
        {
            float d = -10 * deltaTime;
            scene.getEntityByName("main_camera")->getComponent<ComponentTransform>()->move({0.f,0.f,d});
        }
        else if(input.isButtonPressed(Input::ButtonId::S))
        {
            float d = 10 * deltaTime;
            scene.getEntityByName("main_camera")->getComponent<ComponentTransform>()->move({0.f,0.f,d});
        }
        
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