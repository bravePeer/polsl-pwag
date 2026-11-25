add_library(AppResource STATIC
    src/resource.cpp
)

target_include_directories(AppResource PUBLIC include)

target_link_libraries(AppResource
    PUBLIC glm::glm
)