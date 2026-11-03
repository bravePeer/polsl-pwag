#pragma once
#include "resource/resource.hpp"
#include "resource/resource_utils.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <cstdint>

class ResourceMesh : public Resource
{
friend class ResourceManager;
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

    struct ResourceMeshLocation
    {
        vk::DeviceSize vertexOffset;
        vk::DeviceSize vertexSize;
        vk::DeviceSize indexOffset;
        vk::DeviceSize indexSize;
        vk::DeviceSize indexCount;
    };

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

    ResourceMeshLocation& getMeshLocation()
    {
        return meshLocation;
    }

    void setVerticesLocation(vk::DeviceSize vertexOffset, vk::DeviceSize vertexSize)
    {
        meshLocation.vertexOffset = vertexOffset;
        meshLocation.vertexSize = vertexSize;
    }

    void setIndicesLocation(vk::DeviceSize indexOffset, vk::DeviceSize indexSize)
    {
        meshLocation.indexOffset = indexOffset;
        meshLocation.indexSize = indexSize;
        meshLocation.indexCount = indexSize / sizeof(uint32_t);
    }

protected:
    std::unique_ptr<ResourceMeshData>& getMeshData()
    {
        return meshData;
    }

private:
    ResourceMeshLocation meshLocation;
    std::unique_ptr<ResourceMeshData> meshData = nullptr;
    
    bool doLoad() override
    {
        if(!meshData)
        {
            return false;
        }

        return true;
    }

    void doUnload() override
    {
        meshData = nullptr;
    }
};