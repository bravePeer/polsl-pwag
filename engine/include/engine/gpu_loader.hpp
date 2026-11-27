#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <cstdint>
#include <functional>
#include "resource/resource_mesh.hpp"
#include <vector>
#include "engine/context.hpp"
#include "engine/gpu_buffer.hpp"

/// @brief Loads resources to GPU memory
class GpuLoader
{
public:
    explicit GpuLoader(VulkanContext vkContext)
    : vkContext(vkContext)
    { }

    const GpuBuffer& getVertexBuffer() const
    {
        return vertexBuffer;
    }

    const GpuBuffer& getIndexBuffer() const
    {
        return indexBuffer;
    }

    void loadMeshes(std::vector<ResourceMesh*>& meshes)
    {
        if(meshes.empty())
        {
            /// @todo Add log "Nothing laoded"
            return;
        }
        auto [verticiesSize, indiciesSize] = getSizes(meshes);

        createVertexBuffer(meshes, verticiesSize);
        createIndexBuffer(meshes, indiciesSize);
    }


private:
    VulkanContext vkContext;

    /// Resource GPU storage
    GpuBuffer vertexBuffer;
    GpuBuffer indexBuffer;


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

    GpuBuffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        const auto& device = vkContext.device;

        GpuBuffer buffer;

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
        buffer.offset = 0;

        return std::move(buffer);
    }

    void copyBuffer(const vk::raii::Buffer& dstBuffer, const vk::raii::Buffer& srcBuffer, vk::DeviceSize size)
    {
        const auto& vulkanContext = vkContext;

        vk::CommandBufferAllocateInfo allocInfo = {
            .commandPool = vulkanContext.commandPool, 
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        vk::raii::CommandBuffer commandCopyBuffer = std::move(vulkanContext.device.allocateCommandBuffers(allocInfo).front());
        commandCopyBuffer.begin({
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        });
        commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{0, 0, size});
        commandCopyBuffer.end();

        vulkanContext.queue.submit(
            vk::SubmitInfo{
                .commandBufferCount = 1, 
                .pCommandBuffers = &*commandCopyBuffer
            }, 
            nullptr
        );
        vulkanContext.queue.waitIdle();
    }

    /// @brief Stage 0. Calculate sizes
    /// @return sizes of all verticies and indicies as pair<ver, ind>
    std::pair<vk::DeviceSize, vk::DeviceSize> getSizes(const std::vector<ResourceMesh*>& meshes) const
    {
        vk::DeviceSize verticiesSize = 0;
        vk::DeviceSize indiciesSize = 0;

        for(const auto mesh : meshes)
        {
            const auto meshData = mesh->getMeshData();
            if(!meshData)
            {
                /// @todo Maybe log?
                continue;
            }
            verticiesSize += meshData->getVerticesSize();
            indiciesSize += meshData->getIndicesSize();
        }

        return {verticiesSize, indiciesSize};
    }

    /// Stage 1. Create vertex buffer
    void createVertexBuffer(std::vector<ResourceMesh*>& meshes, vk::DeviceSize bufferSize)
    {
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        auto dataStaging = static_cast<uint8_t*>(stagingBuffer.bufferMemory.mapMemory(0, bufferSize));

        vk::DeviceSize offsetBytes = 0;
        for(auto mesh : meshes)
        {
            const auto meshData = mesh->getMeshData();
            if(!meshData)
            {
                continue;
            }

            auto& vertices =  meshData->vertices;
            auto verticesSizeBytes = meshData->getVerticesSize();
            memcpy(dataStaging + offsetBytes, vertices.data(), verticesSizeBytes);

            mesh->setVerticesLocation(
                offsetBytes, 
                offsetBytes / sizeof(Vertex), 
                sizeof(Vertex),
                verticesSizeBytes,
                verticesSizeBytes / sizeof(Vertex)
            );
            offsetBytes += verticesSizeBytes;
        }
        
        stagingBuffer.bufferMemory.unmapMemory();

        vertexBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(vertexBuffer.buffer, stagingBuffer.buffer, bufferSize);
        vertexBuffer.count = static_cast<uint32_t>(offsetBytes);
    }

    /// Stage 2. Create index buffer
    void createIndexBuffer(std::vector<ResourceMesh*>& meshes, vk::DeviceSize bufferSize)
    {
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        auto dataStaging = static_cast<uint8_t*>(stagingBuffer.bufferMemory.mapMemory(0, bufferSize));

        vk::DeviceSize offsetBytes = 0;
        for(auto mesh : meshes)
        {
            const auto meshData = mesh->getMeshData();
            if(!meshData)
            {
                continue;
            }

            auto& indices = meshData->indices;
            auto indicesSizeBytes = meshData->getIndicesSize();
            memcpy(dataStaging + offsetBytes, indices.data(), indicesSizeBytes);

            mesh->setIndicesLocation(
                offsetBytes,
                offsetBytes / sizeof(uint32_t),
                sizeof(uint32_t),
                indicesSizeBytes,
                indicesSizeBytes / sizeof(uint32_t)
            );
            offsetBytes += indicesSizeBytes;
        }
        
        stagingBuffer.bufferMemory.unmapMemory();

        indexBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(indexBuffer.buffer, stagingBuffer.buffer, bufferSize);
        indexBuffer.count = static_cast<uint32_t>(offsetBytes);
    }
};
