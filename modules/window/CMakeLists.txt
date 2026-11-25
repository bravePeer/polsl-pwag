add_library(AppWindow STATIC
    src/window.cpp
    src/window_mapping.cpp
)

target_include_directories(AppWindow 
    PUBLIC include
    PRIVATE include_private
)

target_link_libraries(AppWindow
    PUBLIC Vulkan::Vulkan 
    PUBLIC glfw
    PUBLIC AppInput
    PUBLIC AppEvent
)