#pragma once
#include "component/component.hpp"
#include "component/component_type_id.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

/// @brief Container for components
class Entity
{
public:
    template <typename T, typename... Args>
    T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must be derive from Component");
        
        TypeId typeId = Component::getTypeId<T>();
        auto it = componentsMap.find(typeId);
        if(it != componentsMap.end())
        {
            return static_cast<T*>(it->second);
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setOwner(this);
        T* ptr = component.get();
        componentsMap[typeId] = ptr;
        components.push_back(std::move(component));
        return ptr;
    }

    template <typename T>
    bool removeComponent()
    {
        TypeId typeId = Component::getTypeId<T>();
        auto it = componentsMap.find(typeId);

        if(it != componentsMap.end())
        {
            Component* ptr = it->second;
            componentsMap.erase(it->first);

            for(auto compIt = components.begin(); compIt != components.end(); ++compIt)
            {
                if(compIt->get() == ptr)
                {
                    components.erase(compIt);
                    return true;
                }
            }
        }
        
        return false;
    }

    template <typename T>
    T* getComponent()
    {
        TypeId typeId = Component::getTypeId<T>();
        auto it = componentsMap.find(typeId);
        if(it != componentsMap.end())
        {
            return static_cast<T*>(it->second);
        }

        return nullptr;
    }

    std::unordered_map<TypeId, Component*> getComponents() const
    {
        return componentsMap;
    }

    void update(float deltaTime)
    {
        if(!active)
        {
            return;
        }

        for(auto& component : components)
        {
            component->update(deltaTime);
        }
    }

    void render()
    {
        if(!active)
        {
            return;
        }

        for(auto& component : components)
        {
            component->render();
        }
    }

    bool isActive() const
    {
        return active;
    }

    void setActive(bool activee)
    {
        active = activee;
    }

private:
    std::vector<std::unique_ptr<Component>> components;
    std::unordered_map<TypeId, Component*> componentsMap;

    bool active = true;
};