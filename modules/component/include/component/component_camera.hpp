#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "component/component.hpp"
#include "component/component_transform.hpp"
#include "component/entity.hpp"

class ComponentCamera : public Component
{
public:
    void setPerspective(float newFov, float newaspectRatio, float newnearPlane, float newfarPlane)
    {
        fov = newFov;
        aspectRatio = newaspectRatio;
        nearPlane = newnearPlane;
        farPlane = newfarPlane;

        projectionDirty = true;
    }

    glm::mat4 getViewMatrix()
    {
        if(auto transform = getOwner()->getComponent<ComponentTransform>())
        {
            glm::vec3 position = transform->getPosition();
            glm::quat rotation = transform->getRotation();
            
            glm::vec3 forward = rotation * glm::vec3(0.f, 0.f, -1.f);
            glm::vec3 up = rotation * glm::vec3(0.f, 1.f, 0.f);

            viewMatrix = glm::lookAt(position, position + forward, up);
        }

        return viewMatrix;
    }

    glm::mat4 getProjectionMatrix()
    {
        if(projectionDirty)
        {
            projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
            projectionDirty = false;
        }
        return projectionMatrix;
    }

private:
    float fov = glm::radians(45.f) ; // [rad]
    float aspectRatio = 16.f / 9.f;
    float nearPlane = 0.1f;
    float farPlane = 1000.f;

    mutable bool projectionDirty = true;
    mutable glm::mat4 projectionMatrix;
    
    glm::mat4 viewMatrix = glm::mat4(1.f);
};