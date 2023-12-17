add_test(
    NAME world_ffi
    COMMAND
        ${CMAKE_COMMAND} -E env
            "LUA_PATH=${NSEW_BUILD}/../?.lua"
        ${LUAJIT_COMPILED}/src/luajit test_world_ffi.lua
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
