add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/app.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(vulkan_app PRIVATE Engine AppComponent AppResource AppWindow AppInput)
