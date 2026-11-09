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

    uint32_t getUboOffests()
    {
        return uboOffset;
    }
    
    void setOffests(uint32_t uboOffsett)
    {
        uboOffset = uboOffsett;
    }
protected:
    

private:
    ResourceMesh* mesh = nullptr;
    uint32_t uboOffset;
};