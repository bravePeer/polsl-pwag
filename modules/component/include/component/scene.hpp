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

    Entity* getEntityByName(const std::string& name)
    {
        for(const auto& e : entities)
        {
            if(e->getName() == name)
            {
                return e.get();
            }
        }
        return nullptr;
    }

    void initialize() const
    {
        for(const auto& entity : entities)
        {
            const auto& components = entity->getComponents();
            for(const auto& [compId, component] : components)
            {
                component->initialize();
            }
        }
    }

    void update(float deltaTime) const
    {
        for(const auto& entity : entities)
        {
            entity->update(deltaTime);
        }
    }

    void render() const
    {
        for(const auto& entity : entities)
        {
            entity->render();
        }
    }

private:
    std::vector<std::unique_ptr<Entity>> entities;
};