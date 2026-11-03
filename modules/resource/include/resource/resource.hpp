#pragma once
#include <string>
#include <vulkan_engine/vulkan_context.hpp>

extern vk::Device getDevice();

class Resource
{
public:
    enum class State
    {
        NotLoaded,
        Loaded,
        Released,
    };

    explicit Resource(const std::string& nameId, VulkanContext& vulkanContext)
    : nameId(nameId), vulkanContext(vulkanContext)
    { }

    virtual ~Resource()
    { }

    bool load()
    {
        if(state != State::NotLoaded)
        {
            return false;
        }

        if(doLoad())
        {
            state = State::Loaded;
            return true;
        }

        return false;
    }

    void unload()
    {
        if(state != State::Loaded)
        {
            return;
        }
        
        doUnload();
        state = State::Released;
    }

    bool isLoaded() const
    {
        return state == State::Loaded;
    }

    std::string getNameId() const
    {
        return nameId;
    }

protected:
    VulkanContext& vulkanContext;

private:
    std::string nameId;
    State state = State::NotLoaded;

    virtual bool doLoad() = 0;

    virtual void doUnload() = 0;
};