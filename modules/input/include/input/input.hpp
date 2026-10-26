#pragma once
#include <iostream>
#include <GLFW/glfw3.h>

void inputKeyCb(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        std::cout << "Press" << std::endl;   
    }
}

void inputInit(GLFWwindow* window)
{
    glfwSetKeyCallback(window, inputKeyCb);
}