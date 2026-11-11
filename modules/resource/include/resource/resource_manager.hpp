#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cstdint>
#include "resource/resource.hpp"
#include "engine/context.hpp"
#include "resource/resource.hpp"

class ResourceManager;

template <typename T>
class ResourceHandle
{
public:
    ResourceHandle()
    : resourcesManager(nullptr)
    { }

    ResourceHandle(const std::string& nameId, ResourceManager* resourcesManager)
    : nameId(nameId), resourcesManager(resourcesManager)
    { }

    T* get() const;

    bool isValid() const;

    const std::string& getNameId() const
    {
        return nameId;
    }

    T* operator->() const
    {
        return get();
    }

    T& operator*() const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return isValid();
    }

private:
    std::string nameId;
    ResourceManager* resourcesManager;
};

/// @brief 
/// @details Multi stage load
/// 1. Preload add verticies and indiecies to buffer to load - calculate max size
/// 2. Create one Vertex buffer for all meshes using offsets
/// 3. Create one Index buffer for all meshes using offsets
class ResourceManager
{
public:
    ResourceManager(VulkanContext vulkanContext)
    : vulkanContext(vulkanContext)
    { }

    ~ResourceManager()
    {
        unloadAll();
    }

    template <typename T, typename... Args>
    ResourceHandle<T> load(const std::string& nameId, Args&&... args)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        auto& resourcesType = resources[std::type_index(typeid(T))];
        auto it = resourcesType.find(nameId);

        if(it != resourcesType.end())
        {
            refCounts[std::type_index(typeid(T))][it->first].refCount++;
            return ResourceHandle<T>(nameId, this);
        }

        auto resource = std::make_shared<T>(nameId, vulkanContext, std::forward<Args>(args)...);
        if(!resource->load())
        {
            return ResourceHandle<T>();
        }

        resourcesType[nameId] = resource;
        refCounts[std::type_index(typeid(T))][nameId].refCount = 1;
        refCounts[std::type_index(typeid(T))][nameId].resource = resource;

        return ResourceHandle<T>(nameId, this);
    }

    template <typename T>
    T* getResource(const std::string& nameId) const
    {
        auto itm = resources.find(std::type_index(typeid(T)));
        if(itm == resources.end())
        {
            return nullptr;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);

        if(it != resourcesType.end())
        {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;        
    }

    template <typename T>
    bool hasResource(const std::string& nameId) const
    {
        auto itm = resources.find(std::type_index(typeid(T)));
        if(itm == resources.end())
        {
            return false;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);
        return it != resourcesType.end();
    }

    template <typename T>
    void unload(const std::string& nameId)
    {
        auto itm = refCounts.find(std::type_index(typeid(T)));
        if(itm == refCounts.end())
        {
            return;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);
        if(it == resourcesType.end())
        {
            return;
        }

        auto& resourceData = it->second;
        --resourceData.refCount;

        if(resourceData.refCount == 0)
        {
            if(resourceData.resource)
            {
                resourceData.resource->unload();
            }

            auto& resIt = resources.find(std::type_index(typeid(T)));
            if(resIt != resources.end())
            {
                auto& res = resIt->second;
                auto resIt2 = res.find(nameId);
                if(resIt2 != res.end())
                {
                    res.erase(resIt2);
                }
            }

            resourcesType.erase(it);
        }
    }

    void unloadAll()
    {
        for(auto& [resourceTypeId, resourceType] : resources)
        {
            for(auto& [nameId, resource] : resourceType)
            {
                resource->unload();
            }
        }

        resources.clear();
        refCounts.clear();
    }

    void loadToGpu()
    {
        auto sizes = getSizes();
        if(sizes.first == 0 || sizes.second == 0)
        {
            return;
        }
        createVertexBuffer(sizes.first);
        createIndexBuffer(sizes.second);

        // Unload from CPU
        auto meshes = resources.find(std::type_index(typeid(ResourceMesh)));
        if(meshes == resources.end())
        { /// If no mesh resource then nothing to do
            return;
        }
        vk::DeviceSize offset = 0;
        for(auto& it : meshes->second)
        {
            it.second->unload();
        }
    }


    VulkanBuffer& getVertexBuffer()
    {
        return vertexBuffer;
    }

    VulkanBuffer& getIndexBuffer()
    {
        return indexBuffer;
    }

    std::vector<ResourceMesh*> getMeshes()
    {
        std::vector<ResourceMesh*> ret;
        auto itm = resources.find(std::type_index(typeid(ResourceMesh)));
        if(itm == resources.end())
        { /// If no mesh resource then nothing to do
            return ret;
        }

        for(auto& it : itm->second)
        {
            auto mesh = static_cast<ResourceMesh*>(it.second.get());
            ret.push_back(mesh);
        }

        return ret;
    }

private:
    VulkanContext vulkanContext;
    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

    struct ResourceData
    {
        std::shared_ptr<Resource> resource;
        uint32_t refCount = 0;
    };
    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> refCounts;

    /// Resource GPU storage
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;


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

    

    /// @brief Stage 0. Calculate sizes
    /// @return sizes of all verticies and indicies as pair<ver, ind>
    std::pair<vk::DeviceSize, vk::DeviceSize> getSizes()
    {
        vk::DeviceSize verticiesSize = 0;
        vk::DeviceSize indiciesSize = 0;

        auto itm = resources.find(std::type_index(typeid(ResourceMesh)));
        if(itm == resources.end())
        { /// If no mesh resource then nothing to do
            return {0, 0};
        }

        for(auto& it : itm->second)
        {
            auto mesh = static_cast<ResourceMesh*>(it.second.get());
            auto& meshData = mesh->getMeshData();
            verticiesSize += meshData->getVerticesSize();
            indiciesSize += meshData->getIndicesSize();
        }

        return {verticiesSize, indiciesSize};
    }

    /// Stage 1. Create vertex buffer
    void createVertexBuffer(vk::DeviceSize bufferSize)
    {
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        auto dataStaging = static_cast<uint8_t*>(stagingBuffer.bufferMemory.mapMemory(0, bufferSize));
        
        auto meshes = resources.find(std::type_index(typeid(ResourceMesh)));
        if(meshes == resources.end())
        { /// If no mesh resource then nothing to do
            return;
        }

        vk::DeviceSize offsetBytes = 0;
        for(auto& it : meshes->second)
        {
            auto mesh = static_cast<ResourceMesh*>(it.second.get());
            auto& vertices = mesh->getMeshData()->vertices;
            auto verticesSizeBytes = mesh->getMeshData()->getVerticesSize();
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
    void createIndexBuffer(vk::DeviceSize bufferSize)
    {
        auto stagingBuffer = createBuffer(
            bufferSize, 
            vk::BufferUsageFlagBits::eTransferSrc, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        auto dataStaging = static_cast<uint8_t*>(stagingBuffer.bufferMemory.mapMemory(0, bufferSize));

        auto meshes = resources.find(std::type_index(typeid(ResourceMesh)));
        if(meshes == resources.end())
        { /// If no mesh resource then nothing to do
            return;
        }

        vk::DeviceSize offsetBytes = 0;
        for(auto& it : meshes->second)
        {
            auto mesh = static_cast<ResourceMesh*>(it.second.get());
            auto& indices = mesh->getMeshData()->indices;
            auto indicesSizeBytes = mesh->getMeshData()->getIndicesSize();
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

template <typename T>
T* ResourceHandle<T>::get() const
{
    if(!resourcesManager)
    {
        return nullptr;
    }

    return resourcesManager->getResource<T>(nameId);
}

template <typename T>
bool ResourceHandle<T>::isValid() const
{
    return resourcesManager && resourcesManager->hasResource<T>(nameId);
}