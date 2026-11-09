#pragma once
#include "component/component.hpp"
#include "resource/resource_shader.hpp"

class ComponentShader : public Component
{
public:
    void setShader(ResourceShader* newShader)
    {
        shader = newShader;
    }

    ResourceShader* getShader()
    {
        return shader;
    }

protected:
    void setOffests(uint32_t uboOffsett)
    {
        uboOffset = uboOffsett;
    }

private:
    ResourceShader* shader = nullptr;
    uint32_t uboOffset;
};