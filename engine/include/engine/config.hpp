#pragma once
#include <cstdint>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

constexpr bool enableValidationLayers = true;

constexpr uint32_t kMaxFramesInFlight = 1;
constexpr uint32_t kMaxObjects = 1;