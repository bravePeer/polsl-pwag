#pragma once
#include "component/entity.hpp"
#include <vector>
#include <functional>
#include <memory>

class Scene
{
public:
    Entity& createEntity(std::string name = "")
    {
        return *entities.emplace_back(std::make_unique<Entity>(name));
    }

    std::vector<std::unique_ptr<Entity>>& getEntities()
    {
        return entities;
    }

    Entity* getEntityByName(std::string name)
    {
        for(auto& e : entities)
        {
            if(e->getName() == name)
            {
                return e.get();
            }
        }
        return nullptr;
    }

    void update(float deltaTime)
    {
        for(auto& entity : entities)
        {
            entity->update(deltaTime);
        }
    }

    void render()
    {
        for(auto& entity : entities)
        {
            entity->render();
        }
    }

private:
    std::vector<std::unique_ptr<Entity>> entities;
};