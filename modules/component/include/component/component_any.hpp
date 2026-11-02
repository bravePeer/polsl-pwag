#pragma once
#include <functional>
#include "component/component.hpp"

/// @brief Testing purpose
class ComponentAny : public Component
{
public:
    ComponentAny()
    : fun(nullptr)
    { }

    ComponentAny(std::function<void(float)> fun)
    : fun(fun)
    { }

protected:
    void update(float deltaTime) override
    {
        fun(deltaTime);
    }

private:
    std::function<void(float)> fun;
};