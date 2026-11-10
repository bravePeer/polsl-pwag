#pragma once
#include "resource/resource.hpp"
#include <glm/glm.hpp>

struct UboSet0
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct UboSet1
{
    glm::mat4 transform;
};

class ResourceShader : public Resource
{

};