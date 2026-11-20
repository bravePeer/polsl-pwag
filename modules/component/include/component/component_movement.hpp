#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "component/component.hpp"
#include "event/event_listener.hpp"
#include "event/event_bus.hpp"
#include "event/event_dispatcher.hpp"
#include "input/input_enum.hpp"
#include "component/component_transform.hpp"

class CompoenentMovement : public Component, public EventListener
{
public:
    explicit CompoenentMovement()
    {

    }

    ~CompoenentMovement() override
    {
        eventBus.removeListener(this);
    }

    void onInitialize() override
    {
        eventBus.addListener(this);
    }

    void update(float deltaTime) override
    {
        glm::vec3 movement(0.f);

        if(moveForward)
        {
            movement.z -= 1.f;
        }
        if(moveBackward)
        {
            movement.z += 1.f;
        }
        if(moveLeft)
        {
            movement.x -= 1.f;
        }
        if(moveRight)
        {
            movement.x += 1.f;
        }

        if(mouseValid)
        {
            if(auto transform = getOwner()->getComponent<ComponentTransform>())
            {
                const float rotX = (static_cast<float>((mousePosLastX - mousePosX)) / std::numbers::pi_v<float>) * deltaTime * rotationSpeed;
                transform->rotateWorld(rotX, glm::vec3(0.f, 1.f ,0.f));

                const float rotY = (static_cast<float>((mousePosLastY - mousePosY)) / std::numbers::pi_v<float>) * deltaTime * rotationSpeed;
                transform->rotateLocal(rotY, glm::vec3(1.f, 0.f ,0.f));
            }

            mouseValid = false;
            mousePosLastX = mousePosX;
            mousePosLastY = mousePosY;
        }

        if(glm::length(movement) > 0.f)
        {
            if(auto transform = getOwner()->getComponent<ComponentTransform>())
            {
                transform->moveLocal(glm::normalize(movement) * speed * deltaTime);
            }
        }
    }

    void onEvent(const Event& event) override
    {
        EventDispatcher dispatcher(event);
        
        dispatcher.dispatch<EventKeyPress>([this](const EventKeyPress& event){
            switch(static_cast<InputButtonId>(event.getKeyValue()))
            {
            using enum InputButtonId;
            case W:
                moveForward = true;
                break;
            case S:
                moveBackward = true;
                break;
            case A:
                moveLeft = true;
                break;
            case D:
                moveRight = true;
                break;
            default:
                break;
            }
        });

        dispatcher.dispatch<EventKeyRelease>([this](const EventKeyRelease& event){
            switch(static_cast<InputButtonId>(event.getKeyValue()))
            {
            using enum InputButtonId;
            case W:
                moveForward = false;
                break;
            case S:
                moveBackward = false;
                break;
            case A:
                moveLeft = false;
                break;
            case D:
                moveRight = false;
                break;
            default:
                break;
            }
        });
    
        dispatcher.dispatch<EventMousePos>([this](const EventMousePos& event){
            mousePosX = event.getMousePosX();
            mousePosY = event.getMousePosY();
            mouseValid = true;
        });
    }

private:
    EventBus& eventBus = EventBus::getEventBus();

    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;

    bool mouseValid = false;
    double mousePosX;
    double mousePosY;
    double mousePosLastX;
    double mousePosLastY;

    float speed = 5.f;
    float rotationSpeed = 1.f;

};