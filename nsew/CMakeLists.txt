file(GLOB_RECURSE LUA_FILES CONFIGURE_DEPENDS
    "nsew/*.lua"
    "examples/nsew_client/*.lua"
)

add_subdirectory(nsew)
add_subdirectory(native)
add_subdirectory(examples)
