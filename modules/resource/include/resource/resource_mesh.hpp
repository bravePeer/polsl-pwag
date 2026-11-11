#pragma once
#include "resource/resource.hpp"
#include "resource/resource_utils.hpp"
// #include <vulkan/vulkan_raii.hpp>
#include <cstdint>
#include "resource/vertex.hpp"

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
        uint64_t vertexOffsetBytes; // Offset in bytes
        uint64_t vertexOffset;      // Offset of vertex objects is a number of objects
        uint64_t vertexSize;        // Size of vertex objects in bytes
        uint64_t verticesSize;        // Size of vertex objects in bytes
        uint64_t verticesCount;       // Count of vertex objects

        uint64_t indexOffsetBytes;  // Offset in bytes
        uint64_t indexOffset;       // Offset is a number of objects
        uint64_t indexSize;         // Size of one index object in bytes
        uint64_t indicesSize;       // Size of index objects
        uint64_t indicesCount;      // Count of index objects
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

    void setVerticesLocation(uint64_t vertexOffsetBytes, uint64_t vertexOffset, uint64_t vertexSize, uint64_t verticesSize, uint64_t verticesCount)
    {
        meshLocation.vertexOffsetBytes = vertexOffsetBytes;
        meshLocation.vertexOffset = vertexOffset;
        meshLocation.vertexSize = vertexSize;
        meshLocation.verticesSize = verticesSize;
        meshLocation.verticesCount = verticesCount;
    }

    void setIndicesLocation(uint64_t indexOffsetBytes, uint64_t indexOffset, uint64_t indexSize, uint64_t indicesSize, uint64_t indicesCount)
    {
        meshLocation.indexOffsetBytes = indexOffsetBytes;
        meshLocation.indexOffset = indexOffset;
        meshLocation.indexSize = indexSize;
        meshLocation.indicesSize = indicesSize;
        meshLocation.indicesCount = indicesCount;
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