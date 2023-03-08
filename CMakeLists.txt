cmake_minimum_required(VERSION 3.22)

project(NoitaPatcher
    VERSION 0.1.0
    LANGUAGES CXX C
)

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


add_subdirectory(src)
add_subdirectory(packaging)
