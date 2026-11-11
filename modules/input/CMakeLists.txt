add_library(AppInput STATIC
    src/input.cpp
)

target_include_directories(AppInput PUBLIC include)

target_link_libraries(AppInput
    PRIVATE glfw
    PUBLIC Vulkan::Vulkan
)