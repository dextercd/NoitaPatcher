set(NSEW_BUILD "${CMAKE_CURRENT_BINARY_DIR}/nsew")
set(NSEW_BUILD "${NSEW_BUILD}" PARENT_SCOPE)

file(REMOVE_RECURSE "${NSEW_BUILD}")
file(MAKE_DIRECTORY "${NSEW_BUILD}")

set(SOURCES
    native_dll.lua world_ffi.lua world.lua rect.lua
)

foreach(source IN LISTS SOURCES)
    configure_file(${source} "${NSEW_BUILD}" COPYONLY)
endforeach()

install(DIRECTORY "${NSEW_BUILD}"
    DESTINATION NoitaPatcher/noitapatcher
    COMPONENT NSEW_Library
)

if (WIN32)
    add_library(nsew_native MODULE
        rectangle_optimiser.cpp
    )

    target_link_libraries(nsew_native PRIVATE nsew_common)

    include(GenerateExportHeader)
    generate_export_header(nsew_native)
    target_include_directories(nsew_native PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

    install(TARGETS nsew_native
        DESTINATION NoitaPatcher/noitapatcher/nsew
        COMPONENT NSEW_Library
    )
endif()

if (BUILD_TESTING AND WIN32)
    add_subdirectory(test)
endif()
