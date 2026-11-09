#pragma once
#include <vulkan/vulkan_raii.hpp>

struct VulkanContext
{
    vk::raii::Device& device;
    vk::raii::PhysicalDevice& physicalDevice;
    vk::raii::Queue& queue;
    vk::raii::CommandPool& commandPool;
    // vk::raii::CommandBuffer& commandBuffer;
};