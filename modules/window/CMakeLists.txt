add_library(AppWindow STATIC
    src/window.cpp
)

target_include_directories(AppWindow PUBLIC include)

target_link_libraries(AppWindow
    PRIVATE Vulkan::Vulkan 
    PUBLIC glfw
    PUBLIC AppInput
    PUBLIC Engine
)