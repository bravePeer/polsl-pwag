cmake_minimum_required(VERSION 3.29)

# Add option to enable/disable C++ 20 module
option(ENABLE_CPP20_MODULE "Enable C++ 20 module support for Vulkan" OFF)

# vulkan.cppm uses `import std;`, which is still experimental in CMake 4.0
# (the UUID is specific to the CMake version and must be set before project())
if(ENABLE_CPP20_MODULE)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
endif()

project(vulkan_app LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

# Enable C++ module dependency scanning only if C++ 20 module is enabled
if(ENABLE_CPP20_MODULE)
  set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(app)
add_subdirectory(vulkan_engine)
add_subdirectory(modules)

# add_executable(vulkan_app main.cpp)
target_compile_definitions(vulkan_app PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_compile_options(vulkan_app PRIVATE
    -Wall 
    -Wextra 
    -Wpedantic 
    # -Werror
)

target_link_libraries(vulkan_app PRIVATE glfw)

# set up Vulkan C++ module (enabled when ENABLE_CPP20_MODULE=ON)
if(ENABLE_CPP20_MODULE)
    add_library(VulkanCppModule)
    add_library(Vulkan::cppm ALIAS VulkanCppModule)

    target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
        VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
    )

    target_include_directories(VulkanCppModule PRIVATE "${Vulkan_INCLUDE_DIR}")

    target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)

    # `import std;` requires C++23
    target_compile_features(VulkanCppModule PUBLIC cxx_std_23)
    set_target_properties(VulkanCppModule PROPERTIES CXX_MODULE_STD ON)
    target_sources(VulkanCppModule
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS "${Vulkan_INCLUDE_DIR}"
        FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
    )

    target_compile_definitions(vulkan_app PRIVATE USE_CPP20_MODULES)
    set_target_properties(vulkan_app PROPERTIES CXX_MODULE_STD ON)
    target_link_libraries(vulkan_app PRIVATE Vulkan::cppm)
else()
    target_link_libraries(vulkan_app PRIVATE Vulkan::Vulkan)
    target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
        VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE vulkan_engine::vulkan_engine)

# Compiling shaders
function (add_slang_shader_target TARGET)
    cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
    set (SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
    set (ENTRY_POINTS -entry vertMain -entry fragMain)
    add_custom_command (
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    add_custom_command (
        OUTPUT  ${SHADERS_DIR}/slang.spv
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
        COMMENT "Compiling Slang Shaders"
        VERBATIM
    )
    add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES ${CMAKE_CURRENT_LIST_DIR}/shaders/shader_with_buffers.slang)
add_slang_shader_target(shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(vulkan_app shaders)