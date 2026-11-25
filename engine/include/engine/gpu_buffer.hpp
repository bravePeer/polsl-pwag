#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <cstdint>

struct GpuBuffer
{
    vk::raii::DeviceMemory bufferMemory = nullptr;
    vk::raii::Buffer buffer = nullptr;
    vk::DeviceSize offset = 0;
    uint32_t count = 0;
    void* mapped = nullptr;
};
