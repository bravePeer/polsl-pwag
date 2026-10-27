#pragma once
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan_engine/vulkan_context.hpp>

extern vk::Device getDevice();

class Resource
{
public:
    explicit Resource(const std::string& nameId, VulkanContext& vulkanContext)
    : nameId(nameId), vulkanContext(vulkanContext)
    { }

    virtual ~Resource()
    { }

    bool load()
    { 
        loaded = doLoad();
        return loaded;
    }

    void unload()
    { 
        doUnload();
        loaded = false;
    }

    bool isLoaded() const
    {
        return loaded;
    }

    std::string getNameId() const
    {
        return nameId;
    }

protected:
    VulkanContext& vulkanContext;

private:
    std::string nameId;
    bool loaded = false;

    virtual bool doLoad() = 0;

    virtual void doUnload() = 0;
};

class ResourceMesh : public Resource
{
public:
    struct ResourceMeshData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        uint64_t getVerticesSize()
        {
            return sizeof(Vertex) * vertices.size();
        }

        uint64_t getIndicesSize()
        {
            return sizeof(uint32_t) * indices.size();
        }
    };

    struct VulkanBuffer
    {
        vk::raii::DeviceMemory bufferMemory = nullptr;
        vk::raii::Buffer buffer = nullptr;
        vk::DeviceSize offset = 0;
        uint32_t count = 0;
    };

    /// Temporary in comment
    // explicit ResourceMesh(const std::string& nameId)
    // : Resource(nameId)
    // { }

    explicit ResourceMesh(const std::string& nameId, VulkanContext& vulkanContext, std::unique_ptr<ResourceMeshData> meshData)
    : Resource(nameId, vulkanContext), meshData(std::move(meshData))
    { }

    ~ResourceMesh()
    {
        if(isLoaded())
        {
            doUnload();
        }
    }

private:


    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    std::unique_ptr<ResourceMeshData> meshData = nullptr;

    bool doLoad() override
    {
        if(!meshData)
        {
            return false;
        }

        createVertexBuffer();
        createIndexBuffer();

        return true;
    }

    void doUnload() override
    {

    }


    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = vulkanContext.physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VulkanBuffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        VulkanBuffer buffer;

        vk::BufferCreateInfo bufferInfo{
            .size = size, 
            .usage = usage, 
            .sharingMode = vk::SharingMode::eExclusive
        };
        buffer.buffer = vk::raii::Buffer(vulkanContext.device, bufferInfo);

        vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };
        buffer.bufferMemory = vk::raii::DeviceMemory(vulkanContext.device, allocInfo);
        buffer.buffer.bindMemory(*buffer.bufferMemory, 0);
        buffer.offset = 0;

        return std::move(buffer);
    }

    void createVertexBuffer()
    {
        vk::DeviceSize bufferSize = meshData->getVerticesSize();
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        void *dataStaging = stagingBuffer.bufferMemory.mapMemory(0, bufferSize);
        memcpy(dataStaging, meshData->vertices.data(), bufferSize);
        stagingBuffer.bufferMemory.unmapMemory();

        vertexBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(vertexBuffer.buffer, stagingBuffer.buffer, bufferSize);
        vertexBuffer.count = static_cast<uint32_t>(meshData->vertices.size());
    }

    void createIndexBuffer()
    {
        vk::DeviceSize bufferSize = meshData->getIndicesSize();
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        void* data = stagingBuffer.bufferMemory.mapMemory(0, bufferSize);
        memcpy(data, meshData->indices.data(), bufferSize);
        stagingBuffer.bufferMemory.unmapMemory();

        indexBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(indexBuffer.buffer, stagingBuffer.buffer, bufferSize);
        indexBuffer.count = static_cast<uint32_t>(meshData->indices.size());
    }

    void copyBuffer(const vk::raii::Buffer& dstBuffer, const vk::raii::Buffer& srcBuffer, vk::DeviceSize size)
    {
        vk::CommandBufferAllocateInfo allocInfo{ 
            .commandPool = vulkanContext.commandPool, 
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        vk::raii::CommandBuffer commandCopyBuffer = std::move(vulkanContext.device.allocateCommandBuffers(allocInfo).front());
        commandCopyBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{0, 0, size});
        commandCopyBuffer.end();

        vulkanContext.queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
        vulkanContext.queue.waitIdle();
    }

};

class ResourceShader : public Resource
{

};