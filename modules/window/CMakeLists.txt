add_library(AppWindow STATIC
    src/window.cpp
    src/window_mapping.cpp
)

target_include_directories(AppWindow 
    PUBLIC include
    PRIVATE include_private
)

target_link_libraries(AppWindow
    PRIVATE Vulkan::Vulkan 
    PRIVATE glfw
    PUBLIC AppInput
    PUBLIC Engine
    PUBLIC AppEvent
)