#pragma once
#include <vector>
#include "component/component_camera.hpp"
#include "component/component_mesh.hpp"
#include "component/component_transform.hpp"
#include "component/scene.hpp"
#include "component/entity.hpp"

class SceneCulling
{
public:

    void setCamera(ComponentCamera* newCamera)
    {
        if(!newCamera)
        {
            throw std::runtime_error("Camera can not be null!");
        }

        camera = newCamera;
    }

    std::vector<Entity*> getVisibleEntities(Scene& scene)
    {
        auto& sceneEntities = scene.getEntities();
        std::vector<Entity*> ret;

        for(auto& entity : sceneEntities)
        {
            if(!entity->isActive())
            {
                continue;
            }

            /// Nothing to render
            if(!entity->getComponent<ComponentMesh>())
            {
                continue;
            }

            /// Transform is needed for rendering
            if(!entity->getComponent<ComponentTransform>())
            {
                continue;
            }

            /// If is visible
            if(isVisible(*entity))
            {
                ret.push_back(entity.get());
            }
        }

        return ret;
    }


private:
    ComponentCamera* camera = nullptr;

    bool isVisible(Entity& entity)
    {
        /// @todo
        return true;
    }
};