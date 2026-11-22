#pragma once
#include <optional>
#include "input/input_enum.hpp"

std::optional<InputButtonId> glfwKeyToButtonId(int glfwKey);

std::optional<InputButtonId> glfwMouseButtonToButtonId(int glfwButton);

InputAction glfwActionToInputAction(int action);
