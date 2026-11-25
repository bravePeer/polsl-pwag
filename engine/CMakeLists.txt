add_library(Engine STATIC
    src/engine.cpp
)

target_include_directories(Engine PUBLIC include)

target_link_libraries(Engine
    PUBLIC Vulkan::Vulkan 
    PUBLIC glm::glm 
    PUBLIC glfw
    PUBLIC AppComponent
    PUBLIC AppResource
)
