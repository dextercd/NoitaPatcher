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

configure_file(noitapatcher.rc.in noitapatcher.rc @ONLY)

add_library(noitapatcher MODULE
    main.cpp
    game_pause.cpp
    lua_util.cpp
    extended_logs.cpp
    iat_hook.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/noitapatcher.rc
)

target_link_libraries(noitapatcher
    PRIVATE
        memory_util
        ${LUA_LIBRARIES}
        vs2013
)

target_include_directories(noitapatcher PRIVATE ${LUA_INCLUDE_DIR})

add_executable(noitadis pattern_test.cpp)
target_link_libraries(noitadis PRIVATE memory_util)

install(
    TARGETS noitapatcher
    COMPONENT NoitaPatcher
    LIBRARY DESTINATION NoitaPatcher
)

if (CMAKE_CXX_LINKER_SUPPORTS_PDB)
    install(
        FILES $<TARGET_PDB_FILE:noitapatcher>
        COMPONENT NoitaPatcher
        DESTINATION NoitaPatcher
        OPTIONAL
    )
endif()
