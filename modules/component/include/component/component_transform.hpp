#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "component/component.hpp"

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

    glm::vec3 getPosition()
    {
        return position;
    }

    glm::quat getRotation()
    {
        return rotation;
    }

    glm::vec3 getScale()
    {
        return scale;
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