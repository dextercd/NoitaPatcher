cmake_minimum_required(VERSION 3.26)

project(NoitaPatcher
    VERSION 1.19.2.0
    HOMEPAGE_URL "https://github.com/dextercd/NoitaPatcher"
    DESCRIPTION "Noita Modding Extensions"
    LANGUAGES CXX C
)

include(CTest)

set(NOITAPATCHER_DOCS OFF BOOL "Try building the Sphinx documentation.")

option(NSEW_EXTERNAL_MAP "Build external map executable" ON)
option(NSEW_INCLUDE_EXAMPLE "Package examples" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VISIBILITY_INLINES_HIDDEN ON)
set(C_VISIBILITY_PRESET hidden)
set(CXX_VISIBILITY_PRESET hidden)

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
    if (MSVC)
        add_link_options(/DEBUG)
    endif()
endif()

# Don't add lib suffix when crosscompiling from Linux
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

# Default to 'DOWNLOAD_EXTRACT_TIMESTAMP TRUE' when available
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Dependencies
include(dependencies.cmake)

# Projects
add_subdirectory(vs2013)
add_subdirectory(src)
add_subdirectory(nsew)

if (NOITAPATCHER_DOCS)
    set(DOC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/Documentation/_build)
    add_subdirectory(Documentation)
endif()

# Packaging
add_subdirectory(packaging)
