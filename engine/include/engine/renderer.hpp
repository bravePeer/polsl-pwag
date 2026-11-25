#pragma once
#include <vector>
#include "engine/context.hpp"
#include "engine/buffer_creator.hpp"
#include "component/entity.hpp"
#include "component/component_mesh.hpp"
#include <vulkan/vulkan_raii.hpp>
#include "resource/resource_manager.hpp"
#include "engine/gpu_loader.hpp"

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

        auto& veticesBuffer = GpuLoader::getGpuLoader().getVertexBuffer();
        auto& indicesBuffer = GpuLoader::getGpuLoader().getIndexBuffer();
        
        commandBuffer.bindVertexBuffers(0, *veticesBuffer.buffer, {0});
        commandBuffer.bindIndexBuffer(*indicesBuffer.buffer, 0, vk::IndexType::eUint32);

        for(auto& meshes : meshesReadyToRender)
        {
            auto& meshLocation = meshes.first->getMeshLocation();

            for(auto& obj : meshes.second)
            {
                const uint32_t offset = obj->getUboOffests();
                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayout, 
                    0,
                    *buffCreator->getDescriptorSets(), 
                    offset
                );
                
                commandBuffer.drawIndexed(
                    meshLocation.indicesCount, 
                    1, 
                    meshLocation.indexOffset, 
                    meshLocation.vertexOffset, 
                    0
                );

            }
        }
    }

    

private:
    VulkanContext vkContext;
    std::unordered_map<ResourceMesh*, std::vector<ComponentMesh*>> meshesReadyToRender;
    ResourceManager* resourceManager = nullptr;
    BufferCreator* buffCreator = nullptr;
};