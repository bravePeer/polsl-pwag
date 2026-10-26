#pragma once
#include <string>
#include <vulkan/vulkan_raii.hpp>

extern vk::Device getDevice();

class Resource
{
public:
    explicit Resource(const std::string& nameId)
    : nameId(nameId)
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

    const std::string getNameId() const
    {
        return nameId;
    }

private:
    std::string nameId;
    bool loaded = false;

    virtual bool doLoad() = 0;

    virtual void doUnload() = 0;
};

class ResourceMesh : public Resource
{
public:
    struct VulkanBuffer
    {
        vk::raii::DeviceMemory bufferMemory = nullptr;
        vk::raii::Buffer buffer = nullptr;
        vk::DeviceSize offset = 0;
        uint32_t count = 0;
    };

    explicit ResourceMesh(const std::string& nameId)
    : Resource(nameId)
    { }

    explicit ResourceMesh(std::string& nameId, std::vector<Vertex> verticies, std::vector<uint32_t> indicies)
    : Resource(nameId)
    { 

    }

    ~ResourceMesh()
    {
        doUnload();
    }

    // void loadMeshData(std::vector<Vertex> verticies, std::vector<uint32_t> indicies)

private:
    vk::Device device;
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;

    bool doLoad() 
    {
        return true;
    }

    void doUnload()
    {

    }


//     uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
//     {
//         vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
// 
//         for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//         {
//             if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
//             {
//                 return i;
//             }
//         }
// 
//         throw std::runtime_error("failed to find suitable memory type!");
//     }
};

class ResourceShader : public Resource
{

};