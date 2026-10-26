#pragma once
#include <vector>
#include <component/entity.hpp>
#include <iostream>

inline void componentsDump(std::vector<Entity>& entities)
{
    for(auto& e : entities)
    {
        auto components = e.getComponents();
        for(auto& c : components)
        {
            std::cout << "id: " << c.first << std::endl;
        }
    }
}