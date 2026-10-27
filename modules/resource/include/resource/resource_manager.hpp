#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cstdint>
#include "resource/resource.hpp"
#include "vulkan_engine/vulkan_context.hpp"

class ResourceManager;

template <typename T>
class ResourceHandle
{
public:
    ResourceHandle()
    : resourcesManager(nullptr)
    { }

    ResourceHandle(const std::string& nameId, ResourceManager* resourcesManager)
    : nameId(nameId), resourcesManager(resourcesManager)
    { }

    T* get() const;

    bool isValid() const;

    const std::string& getNameId() const
    {
        return nameId;
    }

    T* operator->() const
    {
        return get();
    }

    T& operator*() const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return isValid();
    }

private:
    std::string nameId;
    ResourceManager* resourcesManager;
};

class ResourceManager
{
public:
    ResourceManager(VulkanContext vulkanContext)
    : vulkanContext(vulkanContext)
    { }

    ~ResourceManager()
    {
        unloadAll();
    }

    template <typename T, typename... Args>
    ResourceHandle<T> load(const std::string& nameId, Args&&... args)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        auto& resourcesType = resources[std::type_index(typeid(T))];
        auto it = resourcesType.find(nameId);

        if(it != resourcesType.end())
        {
            refCounts[std::type_index(typeid(T))][it->first].refCount++;
            return ResourceHandle<T>(nameId, this);
        }

        auto resource = std::make_shared<T>(nameId, vulkanContext, std::forward<Args>(args)...);
        if(!resource->load())
        {
            return ResourceHandle<T>();
        }

        resourcesType[nameId] = resource;
        refCounts[std::type_index(typeid(T))][nameId].refCount = 1;
        refCounts[std::type_index(typeid(T))][nameId].resource = resource;

        return ResourceHandle<T>(nameId, this);
    }

    template <typename T>
    T* getResource(const std::string& nameId) const
    {
        auto itm = resources.find(std::type_index(typeid(T)));
        if(itm == resources.end())
        {
            return nullptr;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);

        if(it != resourcesType.end())
        {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;        
    }

    template <typename T>
    bool hasResource(const std::string& nameId) const
    {
        auto itm = resources.find(std::type_index(typeid(T)));
        if(itm == resources.end())
        {
            return false;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);
        return it != resourcesType.end();
    }

    template <typename T>
    void unload(const std::string& nameId)
    {
        auto itm = refCounts.find(std::type_index(typeid(T)));
        if(itm == refCounts.end())
        {
            return;
        }

        auto& resourcesType = itm->second;
        auto it = resourcesType.find(nameId);
        if(it == resourcesType.end())
        {
            return;
        }

        auto& resourceData = it->second;
        --resourceData.refCount;

        if(resourceData.refCount == 0)
        {
            if(resourceData.resource)
            {
                resourceData.resource->unload();
            }

            auto& resIt = resources.find(std::type_index(typeid(T)));
            if(resIt != resources.end())
            {
                auto& res = resIt->second;
                auto resIt2 = res.find(nameId);
                if(resIt2 != res.end())
                {
                    res.erase(resIt2);
                }
            }

            resourcesType.erase(it);
        }
    }

    void unloadAll()
    {
        for(auto& [resourceTypeId, resourceType] : resources)
        {
            for(auto& [nameId, resource] : resourceType)
            {
                resource->unload();
            }
        }

        resources.clear();
        refCounts.clear();
    }

private:
    VulkanContext vulkanContext;
    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

    struct ResourceData
    {
        std::shared_ptr<Resource> resource;
        uint32_t refCount = 0;
    };
    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> refCounts;
};

template <typename T>
T* ResourceHandle<T>::get() const
{
    if(!resourcesManager)
    {
        return nullptr;
    }
    // return resourcesManager
    return resourcesManager->getResource<T>(nameId);
}

template <typename T>
bool ResourceHandle<T>::isValid() const
{
    return resourcesManager && resourcesManager->hasResource<T>(nameId);
}