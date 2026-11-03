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

    ComponentAny(const std::function<void(ComponentAny*, float)>& fun)
    : fun(fun)
    { }

protected:
    void update(float deltaTime) override
    {
        fun(this, deltaTime);
    }

private:
    std::function<void(ComponentAny*, float)> fun;
};