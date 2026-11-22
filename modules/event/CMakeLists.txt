add_library(AppEvent STATIC
    src/event_bus.cpp
)

target_include_directories(AppEvent PUBLIC include)

target_link_libraries(AppEvent
    PUBLIC AppInputEnum
)
