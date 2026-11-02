#pragma once
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <memory>

#include "app/app_config.hpp"
#include "app/window.hpp"
#include "vulkan_engine/vulkan_engine.hpp"
#include "vulkan_engine/vulkan_resource_manager.hpp"
#include "vulkan_engine/buffer_creator.hpp"

#include "vertex/object.hpp"

#include "component/entity.hpp"
#include "component/component_spamer.hpp"
#include "component/component_transform.hpp"
#include "component/component_camera.hpp"
#include "component/component_utils.hpp"
#include "component/component_mesh.hpp"
#include "component/component_any.hpp"

#include "resource/resource_manager.hpp"
#include "resource/resource_mesh.hpp"

#include "component/scene.hpp"


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
      buffCreator(vkEngine.getContext())
    {
        vkEngine.setBufferCreator(&buffCreator);
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

        ResourceMesh::ResourceMeshData data{
            .vertices = triangleVertices,
            .indices = triangleIndices
        };
        resourceManager.load<ResourceMesh>("test2", std::make_unique<ResourceMesh::ResourceMeshData>(data));
        auto rMesh = resourceManager.getResource<ResourceMesh>("test2");

        auto& camera = scene.createEntity("main_camera");
        camera.addComponent<ComponentCamera>();
        camera.addComponent<ComponentTransform>();

        auto& spammer = scene.createEntity();
        spammer.addComponent<ComponentSpamer>(1, "Spam");

        auto& spammer2 = scene.createEntity();
        spammer2.addComponent<ComponentSpamer>(2, "Spam2");

        auto& mesh1 = scene.createEntity();
        mesh1.addComponent<ComponentMesh>();
        mesh1.getComponent<ComponentMesh>()->setMesh(rMesh);
        mesh1.addComponent<ComponentTransform>();
        mesh1.addComponent<ComponentTransform>()->setPosition({0.f, -0.5f, -5.5f});

        auto& mesh2 = scene.createEntity();
        mesh2.addComponent<ComponentMesh>();
        mesh2.getComponent<ComponentMesh>()->setMesh(rMesh);
        mesh2.addComponent<ComponentTransform>()->setPosition({0.f, 0.f, -10.5f});

        buffCreator.addToGenerate(scene.getEntities());
        buffCreator.createBuffers();
        buffCreator.createDescriptorSet();
        vkEngine.init();

        /// Add Entity to 
    }
    
    App(const App&) = delete;

    ~App() noexcept
    { }

    void run()
    {
        frameLastTime = std::chrono::steady_clock::now();
        while (!window.shouldClose() || !shouldExit)
        {
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

    Scene scene;

    BufferCreator buffCreator;

    std::vector<Object> objects;

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

        auto camera = scene.getEntityByName("main_camera")->getComponent<ComponentCamera>();
        buffCreator.set(camera->getViewMatrix(), camera->getProjectionMatrix());
    }

    void render()
    {
        std::vector<Entity*> visibleEntities = culling.getVisibleEntities(scene);

        // Draw frame
        if(vkEngine.drawFramePub())
        { /// Recreating swap chan
            window.waitIfMinimize();
            if(window.shouldClose())
            {
                shouldExit = true;
                return;
            }
            vkEngine.recreateSwapChainPub();
        }
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