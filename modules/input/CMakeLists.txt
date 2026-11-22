add_library(AppInputEnum INTERFACE)
target_include_directories(AppInputEnum INTERFACE enum/include)

add_library(AppInput STATIC
    src/input.cpp
)
target_include_directories(AppInput PUBLIC include)
target_link_libraries(AppInput
    PRIVATE glfw
    PUBLIC Vulkan::Vulkan
    PUBLIC AppEvent
    PUBLIC AppInputEnum
)