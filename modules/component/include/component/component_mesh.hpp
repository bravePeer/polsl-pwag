#pragma once
#include "component/component.hpp"
#include "resource/resource_mesh.hpp"

class ComponentMesh : public Component
{
public:
    void setMesh(ResourceMesh* newMesh)
    {
        mesh = newMesh;
    }

    ResourceMesh* getMesh()
    {
        return mesh;
    }

protected:
    void setOffests(uint32_t uboOffsett)
    {
        uboOffset = uboOffsett;
    }

private:
    ResourceMesh* mesh = nullptr;
    uint32_t uboOffset;
};