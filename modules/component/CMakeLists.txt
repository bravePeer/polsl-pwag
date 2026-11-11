add_library(AppComponent STATIC
    src/component.cpp
)

target_include_directories(AppComponent PUBLIC include)

target_link_libraries(AppComponent
    PUBLIC AppResource
    PUBLIC glm::glm
)