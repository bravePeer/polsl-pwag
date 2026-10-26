#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include "vulkan_engine/vulkan_config.hpp"

struct VulkanBuffer
{
    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory bufferMemory = nullptr;
};

struct VulkanMesh
{
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t size;
};

struct VulkanObject
{
    // One per frame in flight
    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

    glm::mat4 model;

    void setModelTransform(glm::mat4 transform)
    {
        model = transform;
    }
};

class VulkanResourceManager
{
public:
    VulkanResourceManager(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, vk::raii::Queue& queue, vk::raii::CommandPool& commandPool)
    : device(device), physicalDevice(physicalDevice), queue(queue), commandPool(commandPool) 
    { 
        createDescriptorSetLayout();
        createDescriptorPool();
    }

    void loadObject(ObjectMesh& object)
    {
        createVertexBuffer(object);
        createIndexBuffer(object);
        createUniformBuffer();
        createDescriptorSet();
    }

    VulkanMesh& getVkMesh()
    {
        return vkMesh;
    }

    VulkanObject& getVkObject()
    {
        return vkObj;
    }

    std::vector<vk::raii::DescriptorSet>& getDescriptorSets()
    {
        return descriptorSets;
    }

    vk::raii::DescriptorSetLayout& getDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }
private:
    vk::raii::Device& device;
    vk::raii::PhysicalDevice& physicalDevice;

    vk::raii::Queue& queue;
    vk::raii::CommandPool& commandPool;

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

    // std::vector<VulkanMesh> meshes;
    VulkanMesh vkMesh{};
    VulkanObject vkObj{};


    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VulkanBuffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        VulkanBuffer buffer;

        vk::BufferCreateInfo bufferInfo{
            .size = size, 
            .usage = usage, 
            .sharingMode = vk::SharingMode::eExclusive
        };
        buffer.buffer = vk::raii::Buffer(device, bufferInfo);

        vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };
        buffer.bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
        buffer.buffer.bindMemory(*buffer.bufferMemory, 0);

        return std::move(buffer);
    }

    void createVertexBuffer(ObjectMesh& object)
    {
        vk::DeviceSize bufferSize = sizeof(Vertex) * object.vertices.size();
        auto stagingBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void *dataStaging = stagingBuffer.bufferMemory.mapMemory(0, bufferSize);
        memcpy(dataStaging, object.vertices.data(), bufferSize);
        stagingBuffer.bufferMemory.unmapMemory();

        vkMesh.vertexBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

        copyBuffer(vkMesh.vertexBuffer.buffer, stagingBuffer.buffer, bufferSize);
    }

    void createIndexBuffer(ObjectMesh& object)
    {
        vk::DeviceSize bufferSize = sizeof(uint32_t) * object.indices.size();

        auto stagingBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = stagingBuffer.bufferMemory.mapMemory(0, bufferSize);
        memcpy(data, object.indices.data(), bufferSize);
        stagingBuffer.bufferMemory.unmapMemory();

        vkMesh.indexBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

        copyBuffer(vkMesh.indexBuffer.buffer, stagingBuffer.buffer, bufferSize);
        vkMesh.size = static_cast<uint32_t>(object.indices.size());
    }

    void createUniformBuffer()
    {
        vkObj.uniformBuffers.clear();
        vkObj.uniformBuffersMemory.clear();
        vkObj.uniformBuffersMapped.clear();

        for (size_t i = 0; i < kMaxFramesInFlight; i++)
        {
            vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
            vk::raii::Buffer buffer({});
            vk::raii::DeviceMemory bufferMemory({});
            auto tmpUniformBuffer = createBuffer(
                bufferSize, 
                vk::BufferUsageFlagBits::eUniformBuffer, 
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );

            void* mapped = tmpUniformBuffer.bufferMemory.mapMemory(0, bufferSize);
            vkObj.uniformBuffers.emplace_back(std::move(tmpUniformBuffer.buffer));
            vkObj.uniformBuffersMemory.emplace_back(std::move(tmpUniformBuffer.bufferMemory));
            vkObj.uniformBuffersMapped.emplace_back(mapped);
        }
    }

    void copyBuffer(vk::raii::Buffer& dstBuffer, vk::raii::Buffer& srcBuffer, vk::DeviceSize size)
    {
        vk::CommandBufferAllocateInfo allocInfo{ 
            .commandPool = commandPool, 
            .level = vk::CommandBufferLevel::ePrimary, 
            .commandBufferCount = 1
        };
        vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());
        commandCopyBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
        commandCopyBuffer.end();

        queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
        queue.waitIdle();
    }


    void createDescriptorSetLayout()
    {
        vk::DescriptorSetLayoutBinding uboLayoutBinding{
		    .binding = 0, 
            .descriptorType = vk::DescriptorType::eUniformBuffer, 
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex
        };
        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = 1, 
            .pBindings = &uboLayoutBinding
        };
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
    }

    void createDescriptorPool()
    {
        std::array poolSize {
            vk::DescriptorPoolSize( 
                vk::DescriptorType::eUniformBuffer, 
                kMaxFramesInFlight * kMaxObjects
            )
        };
        vk::DescriptorPoolCreateInfo poolInfo{ 
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 
            .maxSets = kMaxFramesInFlight * kMaxObjects, 
            .poolSizeCount = static_cast<uint32_t>(poolSize.size()), 
            .pPoolSizes = poolSize.data()
        };
        descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
    }

    void createDescriptorSet()
    {
        std::vector<vk::DescriptorSetLayout> layouts(kMaxFramesInFlight, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = *descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
        };

        descriptorSets = device.allocateDescriptorSets(allocInfo);
        for (size_t i = 0; i < kMaxFramesInFlight; i++)
        {
            vk::DescriptorBufferInfo bufferInfo{
                .buffer = vkObj.uniformBuffers[i], 
                .offset = 0, 
                .range = sizeof(UniformBufferObject)
            };
            vk::WriteDescriptorSet   descriptorWrite{   
                .dstSet          = descriptorSets[i],
                .dstBinding      = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo     = &bufferInfo
            };
            device.updateDescriptorSets(descriptorWrite, {});
        }
    }
};