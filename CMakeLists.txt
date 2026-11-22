cmake_minimum_required(VERSION 3.29)

project(vulkan_app LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
# find_program(SLANGC_EXECUTABLE slangc REQUIRED)

add_compile_definitions(
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLFW_INCLUDE_VULKAN
)

add_compile_options(
    -Wall 
    -Wextra 
    -Wpedantic
    # -Wshadow
    # -Werror
)

add_subdirectory(app)
add_subdirectory(engine)
add_subdirectory(modules)



# Compiling shaders
function (add_slang_shader_target TARGET)
    cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
    set (SHADERS_DIR ${CMAKE_BINARY_DIR}/shaders)
    set (ENTRY_POINTS -entry vertMain -entry fragMain)
    add_custom_command (
        OUTPUT  ${SHADERS_DIR}/slang.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${SHADERS_DIR}/slang.spv
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Compiling Slang Shaders"
        VERBATIM
    )
    add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/shader_with_buffers.slang)
add_slang_shader_target(shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(vulkan_app shaders)