#pragma once

using TypeId = size_t;

class ComponentTypeId
{
public:
    template <typename T>
    static TypeId getTypeId()
    {
        static size_t typeId = nextTypeId++;
        return typeId;
    }

private:
    static size_t nextTypeId;
};

inline size_t ComponentTypeId::nextTypeId = 0;