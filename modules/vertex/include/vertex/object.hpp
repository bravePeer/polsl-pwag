#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cstdint>
#include <utility>

#include "vertex.hpp"

/// Vertices
/// Indices
/// Local transform (position, rotation, scale)


struct ObjectMesh
{
    ObjectMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : vertices(vertices),
      indices(indices)
    { }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint64_t getVerticesSize()
    {
        return sizeof(vertices[0]) * vertices.size();
    }

    uint64_t getIndicesSize()
    {
        return sizeof(indices[0]) * indices.size();
    }
};

struct ObjectTransform
{
    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);

    glm::mat4 getModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotation.x, glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.f, 1.f, 0.f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, scale);
        return model;
    }

    void move(glm::vec3 val)
    {
        position += val;
    }
};

struct Object
{
    Object(ObjectMesh& mesh, ObjectTransform transform)
    : mesh(mesh),
      transform(transform)
    { }

    ObjectMesh& mesh;
    ObjectTransform transform;

    // One per frame in flight
    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

};