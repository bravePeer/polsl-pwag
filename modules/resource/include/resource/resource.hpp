#pragma once
#include <string>
#include <vulkan_engine/vulkan_context.hpp>

extern vk::Device getDevice();

class Resource
{
public:
    explicit Resource(const std::string& nameId, VulkanContext& vulkanContext)
    : nameId(nameId), vulkanContext(vulkanContext)
    { }

    virtual ~Resource()
    { }

    bool load()
    { 
        loaded = doLoad();
        return loaded;
    }

    void unload()
    { 
        doUnload();
        loaded = false;
    }

    bool isLoaded() const
    {
        return loaded;
    }

    std::string getNameId() const
    {
        return nameId;
    }

protected:
    VulkanContext& vulkanContext;

private:
    std::string nameId;
    bool loaded = false;

    virtual bool doLoad() = 0;

    virtual void doUnload() = 0;
};