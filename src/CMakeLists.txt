add_library(noitapatcher MODULE
    main.cpp
    executable_info.cpp
)

target_link_libraries(noitapatcher
    PRIVATE
        minhook
        dbghelp
        ${LUA_LIBRARIES}
)

target_include_directories(noitapatcher PRIVATE ${LUA_INCLUDE_DIR})

install(
    TARGETS noitapatcher
    COMPONENT NoitaPatcher
    LIBRARY DESTINATION .
)
