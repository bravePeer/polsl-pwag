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

    void rotate(float angle, glm::vec3 axis)
    {
        rotation = glm::rotate(rotation, angle, axis);
        dirty = true;
    }

    void rotateWorld(float angle, glm::vec3 axis)
    {
        rotation = glm::normalize(glm::angleAxis(angle, axis) * rotation);
        dirty = true;
    }

    void rotateLocal(float angle, glm::vec3 axis)
    {
        rotation = glm::normalize(rotation * glm::angleAxis(angle, axis));
        dirty = true;
    }

    void move(glm::vec3 distance)
    {
        position += distance;
        dirty = true;
    }

    void moveLocal(glm::vec3 distance)
    {
        position += rotation * distance;
        dirty = true;
    }

    glm::vec3 getForward() const
    {
        return rotation * glm::vec3(0.f, 0.f, -1.f);
    }

    glm::vec3 getRight() const
    {
        return rotation * glm::vec3(1.f, 0.f, 0.f);
    }

    glm::vec3 getUp() const
    {
        return rotation * glm::vec3(0.f, 1.f, 0.f);
    }

    glm::vec3 getPosition() const
    {
        return position;
    }

    glm::quat getRotation() const
    {
        return rotation;
    }

    glm::vec3 getScale() const
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