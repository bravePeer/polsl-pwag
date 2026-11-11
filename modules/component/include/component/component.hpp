#pragma once
#include <cstddef>
#include "component/component_type_id.hpp"

class Entity;

class Component
{
    friend class Entity;
public:
    enum class State
    {
        Uninitialized,
        Initializing,
        Active,
        Destroying,
        Destroyed
    };

    virtual ~Component()
    {
        if(state != State::Destroyed)
        {
            state = State::Destroyed;
        }
    }
    
    void initialize();

    virtual void update(float deltaTime) 
    { }

    virtual void render()
    { }

    void destroy()
    {
        if(state == State::Active)
        {
            state = State::Destroying;
            onDestroy();
            state = State::Destroyed;
        }
    }

    template <typename T>
    static size_t getTypeId()
    {
        return ComponentTypeId::getTypeId<T>();
    }

    Entity* getOwner()
    {
        return owner;
    }

protected:
    void setOwner(Entity* newOwner)
    {
        owner = newOwner;
    }

    // virtual void onUpdate(float deltaTime) 
    // { }

    // virtual void onRender()
    // { }

    virtual void onInitialize()
    { }

    virtual void onDestroy()
    { }

private:
    State state = State::Uninitialized;
    Entity* owner = nullptr;
};