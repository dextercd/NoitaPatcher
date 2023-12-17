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


# Needed for MINGW to find the LuaJIT dll
if (MINGW)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

find_package(Lua REQUIRED)


FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG main
)

# googletest has an optional Python dependency, but since we don't need that
# feature we can disable the search for Python. Searching for Python in my
# Windows VM takes about 20 seconds so this is a nice optimisation.
set(CMAKE_DISABLE_FIND_PACKAGE_Python ON)

FetchContent_MakeAvailable(googletest)


FetchContent_Declare(absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp
    GIT_TAG master
)

set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "")
set(ABSL_USE_EXTERNAL_GOOGLETEST ON CACHE BOOL "")

FetchContent_MakeAvailable(absl)


find_package(Boost 1.78 REQUIRED COMPONENTS system)
