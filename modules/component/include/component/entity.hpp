#pragma once
#include "component/component.hpp"

#include <vector>
#include <memory>

/// @brief Container for components
class Entity
{
public:

    template <typename T, typename... Args>
    T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must be derive from Component");
        
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        components.push_back(std::move(component));
        return ptr;
    }

    template <typename T>
    T* getComponent()
    {
        for(auto& component : components)
        {
            if(T* componentExists = dynamic_cast<T*>(component.get()))
            {
                return componentExists;
            }
        }
        return nullptr;
    }

    void update(float deltaTime)
    {
        for(auto& component : components)
        {
            component.get()->update(deltaTime);
        }
    }

private:
    std::vector<std::unique_ptr<Component>> components;
};