set(CMAKE_CXX_STANDARD 20)

add_library(memory_util STATIC
    executable_info.cpp
    memory_pattern.cpp
)
target_link_libraries(memory_util PRIVATE dbghelp)

add_library(noitapatcher MODULE
    main.cpp
)

target_link_libraries(noitapatcher
    PRIVATE
        minhook
        memory_util
        ${LUA_LIBRARIES}
)

target_include_directories(noitapatcher PRIVATE ${LUA_INCLUDE_DIR})

add_executable(noitadis pattern_test.cpp)
target_link_libraries(noitadis PRIVATE memory_util)

install(
    TARGETS noitapatcher
    COMPONENT NoitaPatcher
    LIBRARY DESTINATION .
)
