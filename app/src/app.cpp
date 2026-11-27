#include "app/app.hpp"
#include "engine/engine.hpp"
#include "resource/resource_models.hpp"
#include "component/component_movement.hpp"
#include "component/component_any.hpp"

void loadScene(VulkanEngine& vkEngine, ResourceManager& resourceManager, Scene& scene)
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
    auto ref = resourceManager.getMeshes();
    vkEngine.loadMeshes(ref);

    auto& camera = scene.createEntity("main_camera");
    camera.addComponent<ComponentCamera>();
    camera.addComponent<ComponentTransform>();
    camera.addComponent<CompoenentMovement>();

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
}

void App::init()
{
    loadScene(vkEngine, resourceManager, scene);

    vkEngine.initBuffers(scene.getEntities());
    vkEngine.init();

    vkEngine.setResourceManager(&resourceManager);

    scene.initialize();
}