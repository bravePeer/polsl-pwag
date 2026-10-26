#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// For spam component
#include <chrono>
#include <iostream>

#include "component/component_type_id.hpp"

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
            onDestroy();
            state = State::Destroyed;
        }
    }
    
    void initialize()
    {
        if(state == State::Uninitialized)
        {
            state = State::Initializing;
            onInitialize();
            state = State::Active;
        }
    }

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

protected:
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
    /// Entity owner is needed?
};

class ComponentTransform : public Component
{
public:
    void setPosition(glm::vec3 newPosition)
    {
        position = newPosition;
        dirty = true;
    }

    void setRotation(glm::quat newRotation)
    {
        rotation = newRotation;
        dirty = true;
    }

    void setScale(glm::vec3 newScale)
    {
        scale = newScale;
        dirty = true;
    }

    glm::mat4 getTransform()
    {
        if(dirty)
        {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

            transform = translationMatrix * rotationMatrix * scaleMatrix;
            dirty = false;
        }

        return transform;
    }
private:
    glm::vec3 position = glm::vec3(0.f);
    glm::quat rotation = glm::quat(1.0f, 0.f, 0.f, 0.f);
    glm::vec3 scale = glm::vec3(1.f);

    /// Czy mutable daje tu jakiekolwiek korzyści?
    // Cached transformation matrix
    mutable glm::mat4 transform;
    mutable bool dirty = true;
};


/// @brief Testing purpose
class ComponentSpamer : public Component
{
public:
    ComponentSpamer()
    : spamPeriod(1.f)
    { }

    ComponentSpamer(float spamPeriod, std::string text)
    : spamPeriod(spamPeriod), textToPrint(text)
    { }

protected:
    void update(float deltaTime) override
    {
        // To też będzie dobrze ale chyba lepiej by było po prostu dodawać deltaTime
        auto timeNow = std::chrono::steady_clock::now();
        auto tmp = std::chrono::duration<float>(timeNow - lastSpamWrite).count();
        if(tmp > spamPeriod)
        {
            std::cout << textToPrint << std::endl;
            lastSpamWrite = timeNow;
        }
    }

private:
    float spamPeriod;
    std::string textToPrint;
    std::chrono::time_point<std::chrono::steady_clock> lastSpamWrite;
};