file(GLOB_RECURSE LUA_FILES CONFIGURE_DEPENDS
    "nsew/*.lua"
    "examples/nsew_client/*.lua"
)

if (NSEW_EXTERNAL_MAP)
    add_subdirectory(external_map)
endif()

add_subdirectory(nsew)
add_subdirectory(native)
add_subdirectory(examples)

#add_subdirectory(packaging)
