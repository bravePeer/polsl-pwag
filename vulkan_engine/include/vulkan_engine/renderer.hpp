#pragma once
#include <vector>
#include "vulkan_engine/vulkan_context.hpp"
#include "vulkan_engine/buffer_creator.hpp"
#include "component/entity.hpp"
#include "component/component_mesh.hpp"
#include <vulkan/vulkan_raii.hpp>
#include "resource/resource_manager.hpp"

class Renderer
{
public:

    Renderer(VulkanContext vkContext)
    : vkContext(vkContext)
    { }

    void setResourceManager(ResourceManager* rm)
    {
        resourceManager = rm;
    }
    
    void setBuffCreator(BufferCreator* bc)
    {
        buffCreator = bc;
    }

    void addEntities(std::vector<Entity*> entitiesToRender)
    {
        meshesReadyToRender.clear();
        for(auto& entity : entitiesToRender)
        {
            auto componenet = entity->getComponent<ComponentMesh>();
            auto resource = componenet->getMesh();
            meshesReadyToRender[resource].push_back(componenet);
        }
    }

    void renderMeshes(vk::raii::CommandBuffer& commandBuffer, vk::raii::PipelineLayout& pipelineLayout)
    {
        if(!resourceManager)
        {
            throw std::runtime_error("ResourceManager not set");
        }

        if(!buffCreator)
        {
            throw std::runtime_error("BuffCreator not set");
        }

        // auto& commandBuffer = vkContext.commandBuffer;

        auto& vetices = resourceManager->getVertexBuffer();
        auto& indices = resourceManager->getIndexBuffer();
        
        commandBuffer.bindVertexBuffers(0, *vetices.buffer, {0});
        commandBuffer.bindIndexBuffer(*indices.buffer, 0, vk::IndexType::eUint32);

        for(auto& meshes : meshesReadyToRender)
        {
            auto& meshLocation = meshes.first->getMeshLocation();

            for(auto& obj : meshes.second)
            {
                // uint32_t offset = buffCreator->offset(0, i);
                const uint32_t offset = obj->getUboOffests();
                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayout, 
                    0,
                    *buffCreator->getDescriptorSets(), 
                    offset
                );
                
                commandBuffer.drawIndexed(meshLocation.indexCount, 1, static_cast<uint32_t>(meshLocation.indexOffset / sizeof(uint32_t)), 0, meshLocation.vertexOffset / sizeof(Vertex));

            }
        }
    }

    

private:
    VulkanContext vkContext;
    std::unordered_map<ResourceMesh*, std::vector<ComponentMesh*>> meshesReadyToRender;
    ResourceManager* resourceManager = nullptr;
    BufferCreator* buffCreator = nullptr;
};