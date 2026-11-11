#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan_engine/vulkan_engine.hpp"
#include "input/input.hpp"

struct WindowDataProvider
{
    VulkanEngine* engine;
    Input* input;
};

struct WindowFunProvider
{
    GLFWframebuffersizefun frameBufferSizeFun;
    GLFWkeyfun keyFun;
    GLFWmousebuttonfun mouseButtonFun;
    GLFWcursorposfun mouseCursorFun;
};