add_library(vulkan_engine INTERFACE)
add_library(vulkan_engine::vulkan_engine ALIAS vulkan_engine)

target_sources(vulkan_engine INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/config.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/shader_loader.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/vulkan_engine.hpp
)
target_compile_features(vulkan_engine INTERFACE cxx_std_20)
target_compile_definitions(vulkan_engine INTERFACE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
target_link_libraries(vulkan_engine INTERFACE Vulkan::Vulkan glm::glm glfw)
if(ENABLE_CPP20_MODULE)
    target_link_libraries(vulkan_engine INTERFACE Vulkan::cppm)
    target_compile_definitions(vulkan_engine INTERFACE USE_CPP20_MODULES)
endif()
