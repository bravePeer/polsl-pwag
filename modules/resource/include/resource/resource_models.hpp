#pragma once
#include "resource/resource_mesh.hpp"

#include <cmath>
#include <cstdint>
#include <numbers>

// Cuboid centered at (0,0,0).
// length -> X axis, height -> Y axis, width -> Z axis.
// 24 vertices (4 per face) so each face can have its own color (and later normals/UVs).
// Triangles are counter-clockwise when the face is viewed from outside.
inline ResourceMesh::ResourceMeshData createCube(float length, float width, float height)
{
    const float x = length * 0.5f;
    const float y = height * 0.5f;
    const float z = width  * 0.5f;

    const glm::vec3 red    {1.0f, 0.0f, 0.0f};
    const glm::vec3 green  {0.0f, 1.0f, 0.0f};
    const glm::vec3 blue   {0.0f, 0.0f, 1.0f};
    const glm::vec3 yellow {1.0f, 1.0f, 0.0f};
    const glm::vec3 cyan   {0.0f, 1.0f, 1.0f};
    const glm::vec3 magenta{1.0f, 0.0f, 1.0f};

    return ResourceMesh::ResourceMeshData{
        .vertices = {
            // Front (+Z)
            {{-x, -y,  z}, red},
            {{ x, -y,  z}, red},
            {{ x,  y,  z}, red},
            {{-x,  y,  z}, red},
            // Back (-Z)
            {{ x, -y, -z}, green},
            {{-x, -y, -z}, green},
            {{-x,  y, -z}, green},
            {{ x,  y, -z}, green},
            // Left (-X)
            {{-x, -y, -z}, blue},
            {{-x, -y,  z}, blue},
            {{-x,  y,  z}, blue},
            {{-x,  y, -z}, blue},
            // Right (+X)
            {{ x, -y,  z}, yellow},
            {{ x, -y, -z}, yellow},
            {{ x,  y, -z}, yellow},
            {{ x,  y,  z}, yellow},
            // Top (+Y)
            {{-x,  y,  z}, cyan},
            {{ x,  y,  z}, cyan},
            {{ x,  y, -z}, cyan},
            {{-x,  y, -z}, cyan},
            // Bottom (-Y)
            {{-x, -y, -z}, magenta},
            {{ x, -y, -z}, magenta},
            {{ x, -y,  z}, magenta},
            {{-x, -y,  z}, magenta},
        },
        .indices = {
             0,  1,  2,   2,  3,  0, // front
             4,  5,  6,   6,  7,  4, // back
             8,  9, 10,  10, 11,  8, // left
            12, 13, 14,  14, 15, 12, // right
            16, 17, 18,  18, 19, 16, // top
            20, 21, 22,  22, 23, 20, // bottom
        }
    };
}

// Cone centered at (0,0,0), axis along Y.
// Base (circle of given radius) at y = -height/2, apex at y = +height/2.
// segments -> number of sides approximating the circle (min 3).
// Each side triangle has its own 3 vertices so it can have its own color (flat look, like the cube faces).
// The base is a triangle fan around a center vertex.
// Triangles are counter-clockwise when viewed from outside.
inline ResourceMesh::ResourceMeshData createCone(float radius, float height, uint32_t segments = 32)
{
    if (segments < 3)
        segments = 3;

    const float y = height * 0.5f;
    const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);

    const glm::vec3 baseColor{1.0f, 0.0f, 1.0f}; // magenta

    // Point on the ring: angle 0 -> +Z, increasing angle -> towards +X.
    auto ringPoint = [&](uint32_t i, float py) {
        const float a = step * static_cast<float>(i % segments);
        return glm::vec3{radius * std::sin(a), py, radius * std::cos(a)};
    };

    ResourceMesh::ResourceMeshData data;
    data.vertices.reserve(segments * 3 + 1 + segments);
    data.indices.reserve(segments * 3 * 2);

    // Sides
    for (uint32_t i = 0; i < segments; ++i)
    {
        const float mid = step * (static_cast<float>(i) + 0.5f);
        const glm::vec3 color{
            0.5f + 0.5f * std::cos(mid),
            0.5f + 0.5f * std::sin(mid),
            0.5f - 0.5f * std::cos(mid),
        };

        const auto first = static_cast<uint32_t>(data.vertices.size());
        data.vertices.push_back({ringPoint(i,     -y), color});
        data.vertices.push_back({ringPoint(i + 1, -y), color});
        data.vertices.push_back({{0.0f, y, 0.0f},      color}); // apex

        data.indices.insert(data.indices.end(), {first, first + 1, first + 2});
    }

    // Base (-Y)
    const auto center = static_cast<uint32_t>(data.vertices.size());
    data.vertices.push_back({{0.0f, -y, 0.0f}, baseColor});
    for (uint32_t i = 0; i < segments; ++i)
        data.vertices.push_back({ringPoint(i, -y), baseColor});

    for (uint32_t i = 0; i < segments; ++i)
    {
        const uint32_t current = center + 1 + i;
        const uint32_t next    = center + 1 + (i + 1) % segments;
        data.indices.insert(data.indices.end(), {center, next, current});
    }

    return data;
}
