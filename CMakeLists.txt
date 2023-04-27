cmake_minimum_required(VERSION 3.22)

set(NOITAPATCHER_DOCS OFF BOOL "Try building the Sphinx documentation.")

project(NoitaPatcher
    VERSION 1.8.1
    LANGUAGES CXX C
)

# Needed for MINGW to find the LuaJIT dll
if (MINGW)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

# Don't add lib suffix when crosscompiling from Linux
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

# Default to 'DOWNLOAD_EXTRACT_TIMESTAMP TRUE' when available
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(MinHook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG 49d03ad118cf7f6768c79a8f187e14b8f2a07f94)
FetchContent_MakeAvailable(MinHook)


FetchContent_Declare(Zydis
    GIT_REPOSITORY https://github.com/zyantific/zydis.git
    GIT_TAG 4a661c659edaa51c3a459a885add92a216c87612
)
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
option(ZYDIS_BUILD_DOXYGEN "" OFF)
FetchContent_MakeAvailable(Zydis)

find_package(Lua REQUIRED)

add_subdirectory(src)
if (NOITAPATCHER_DOCS)
    add_subdirectory(Documentation)
endif()
add_subdirectory(packaging)