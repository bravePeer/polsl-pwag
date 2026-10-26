#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// For spam component
#include <chrono>
#include <iostream>

class Component
{
public:
    virtual ~Component() = default;
    virtual void update(float deltaTime) 
    { }
private:
};

class ComponentTransform : public Component
{
public:
    void setPosition(glm::vec3 newPosition)
    {
        position = newPosition;
        dirty = true;
    }

    void setRotation(glm::vec3 newRotation)
    {
        rotation = newRotation;
        dirty = true;
    }

    void setScale(glm::vec3 newScale)
    {
        scale = newScale;
        dirty = true;
    }

    void update(float deltaTime) override
    {
        if(dirty)
        {
            glm::mat4 model = glm::mat4(1.f);
            model = glm::translate(model, position);
            model = glm::rotate(model, rotation.x, glm::vec3(1.f, 0.f, 0.f));
            model = glm::rotate(model, rotation.y, glm::vec3(0.f, 1.f, 0.f));
            model = glm::rotate(model, rotation.z, glm::vec3(0.f, 0.f, 1.f));
            model = glm::scale(model, scale);
            dirty = false;
        }
    }
private:
    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
    glm::mat4 model;

    bool dirty = true;
};

class ComponentSpamer : public Component
{
public:
    ComponentSpamer()
    : spamPeriod(1.f)
    { }

    ComponentSpamer(float spamPeriod, std::string text)
    : spamPeriod(spamPeriod), textToPrint(text)
    { }

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