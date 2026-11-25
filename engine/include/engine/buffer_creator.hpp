#pragma once 
#include "engine/context.hpp"
#include "component/entity.hpp"
#include "component/component_mesh.hpp"
#include "component/component_transform.hpp"
#include "resource/resource.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include "engine/gpu_buffer.hpp"
#include "engine/ubo.hpp"



/// @brief Create VMA (Vulkan Memory Allocator)
class BufferCreator
{
public:
    BufferCreator(VulkanContext vkContext)
    : vkContext(vkContext)
    { }


    void addToGenerate()
    {
        /// @todo Finish implementation    
    }
    
    void addToGenerate(std::vector<std::unique_ptr<Entity>>& entities)
    {
        size_t i = 0;
        for(auto& entity : entities)
        {
            auto mesh = entity->getComponent<ComponentMesh>();
            auto transform = entity->getComponent<ComponentTransform>();
            if(mesh && transform)
            {
                ObjSomeStruct tmp = {
                    .mesh = mesh,
                    .transform = transform,
                    .index = i
                };
                components.push_back(tmp);
                i++;
            }
        }
    }

    GpuBuffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        vk::BufferCreateInfo bufferInfo{
            .size = size, 
            .usage = usage, 
            .sharingMode = vk::SharingMode::eExclusive
        };
        buffer.buffer = vk::raii::Buffer(vkContext.device, bufferInfo);

        vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };
        buffer.bufferMemory = vk::raii::DeviceMemory(vkContext.device, allocInfo);
        buffer.buffer.bindMemory(*buffer.bufferMemory, 0);

        return std::move(buffer);
    }

    void createDescriptorSet()
    {
        vk::DescriptorSetLayoutBinding uboLayoutBinding{
		    .binding = 0, 
            .descriptorType = vk::DescriptorType::eUniformBufferDynamic, 
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex
        };

        vk::DescriptorPoolSize poolSize = {
            .type = vk::DescriptorType::eUniformBufferDynamic, 
            .descriptorCount = 1
        };
        vk::DescriptorPoolCreateInfo poolInfo{ 
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 
            .maxSets = 1, 
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };
        descriptorPool = vk::raii::DescriptorPool(vkContext.device, poolInfo);

        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = 1, 
            .pBindings = &uboLayoutBinding
        };
        descriptorSetLayout = vk::raii::DescriptorSetLayout(vkContext.device, layoutInfo);

        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = *descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &(*descriptorSetLayout)
        };

        descriptorSets = std::move(vkContext.device.allocateDescriptorSets(allocInfo).front());
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = buffer.buffer, 
            .offset = 0, 
            .range = sizeof(UniformBufferObject)
        };
        vk::WriteDescriptorSet descriptorWrite{   
            .dstSet          = *descriptorSets,
            .dstBinding      = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = vk::DescriptorType::eUniformBufferDynamic,
            .pBufferInfo     = &bufferInfo
        };
        vkContext.device.updateDescriptorSets(descriptorWrite, {});


    }

    void createBuffers()
    {
        // create vertex buffers for all meshes
        // create indicies buffers for all meshes
        // create UBOs for all entities

        size_t align = vkContext.physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment;
        stride = (sizeof(UniformBufferObject) + align - 1) & ~(align - 1);
        size_t bufferSize = stride * components.size() * kMaxFramesInFlight;
        buffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eUniformBuffer, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        buffer.mapped = buffer.bufferMemory.mapMemory(0, bufferSize);
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = vkContext.physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    uint32_t offset(uint32_t frame, uint32_t obj) const 
    {
        return static_cast<uint32_t>((frame * components.size() + obj) * stride);
    }

    void update(uint32_t frame, uint32_t obj, const UniformBufferObject& data) 
    {
        std::memcpy(static_cast<uint8_t*>(buffer.mapped) + offset(frame, obj), &data, sizeof(data));
    }

    void set(glm::mat4 const& tview, glm::mat4 tproj)
    {
        view = tview;
        proj = tproj;
    }

    void updateUbo(uint32_t frame)
    {
        for(size_t i = 0; i < components.size(); i++)
        {
            UniformBufferObject ubo = {
                .model = components[i].transform->getTransform(),
                .view = view,
                .proj = proj
            };
            update(frame, i, ubo);
        }
    }

    size_t getSize()
    {
        return components.size();
    }

    vk::raii::DescriptorSet& getDescriptorSets()
    {
        return descriptorSets;
    }

    vk::raii::DescriptorSetLayout& getDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }

    void bindUboOffsets()
    {
        for(size_t i = 0; i < components.size(); i++)
        {
            components[i].mesh->setOffests(i * stride * kMaxFramesInFlight);
        }
    }
private:
    struct ObjSomeStruct
    {
        ComponentMesh* mesh;
        ComponentTransform* transform;
        size_t index;
    };

    VulkanContext vkContext;
    std::vector<ObjSomeStruct> components;

    size_t stride;
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    GpuBuffer buffer;

    glm::mat4 view;
    glm::mat4 proj;

    vk::raii::DescriptorSet descriptorSets = nullptr;
};