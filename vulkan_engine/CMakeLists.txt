add_library(vulkan_engine INTERFACE)
add_library(vulkan_engine::vulkan_engine ALIAS vulkan_engine)

target_sources(vulkan_engine INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/vulkan_config.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/vulkan_shader_loader.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_engine/vulkan_engine.hpp
)
target_compile_features(vulkan_engine INTERFACE cxx_std_20)
target_compile_definitions(vulkan_engine INTERFACE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)
target_link_libraries(vulkan_engine INTERFACE Vulkan::Vulkan glm::glm glfw)
if(ENABLE_CPP20_MODULE)
    target_link_libraries(vulkan_engine INTERFACE Vulkan::cppm)
    target_compile_definitions(vulkan_engine INTERFACE USE_CPP20_MODULES)
endif()
