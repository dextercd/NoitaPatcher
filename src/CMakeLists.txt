set(CMAKE_CXX_STANDARD 20)

add_library(memory_util STATIC
    executable_info.cpp
    memory_pattern.cpp
    x86.cpp
)
target_link_libraries(memory_util
    PRIVATE
        dbghelp
    PUBLIC
        Zydis
        minhook
)

add_library(noitapatcher MODULE
    main.cpp
    game_pause.cpp
    lua_util.cpp
    extended_logs.cpp
    iat_hook.cpp
)

target_link_libraries(noitapatcher
    PRIVATE
        memory_util
        ${LUA_LIBRARIES}
)

target_include_directories(noitapatcher PRIVATE ${LUA_INCLUDE_DIR})

add_executable(noitadis pattern_test.cpp)
target_link_libraries(noitadis PRIVATE memory_util)

install(
    TARGETS noitapatcher
    COMPONENT NoitaPatcher
    LIBRARY DESTINATION NoitaPatcher
)
