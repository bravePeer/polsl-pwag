#pragma once
#include <GLFW/glfw3.h>
#include "engine/engine.hpp"
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