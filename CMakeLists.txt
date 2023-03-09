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
include(ExternalProject)

FetchContent_Declare(MinHook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG 49d03ad118cf7f6768c79a8f187e14b8f2a07f94)
FetchContent_MakeAvailable(MinHook)


set(LUAJIT_OUTPUT luajit.exe lua51.dll)
if (MSVC)
    set(LUAJIT_BUILD ${CMAKE_COMMAND} -E chdir "<SOURCE_DIR>/src" "msvcbuild.bat")
    list(APPEND LUAJIT_OUTPUT lua51.lib)
else()
    # Not MSVC so try the Makefile instead
    find_program(MAKE_EXECUTABLE NAMES gmake make)
    set(LUAJIT_BUILD
        ${MAKE_EXECUTABLE}
            CC=${CMAKE_C_COMPILER}
            "CFLAGS=-static"
            "LDFLAGS=-static"
            TARGET_SYS=Windows
            MINILUA_T=host/minilua.exe
            BUILDVM_T=host/buildvm.exe
            "MINILUA_X=${CMAKE_CROSSCOMPILING_EMULATOR} host/minilua.exe"
            "BUILDVM_X=${CMAKE_CROSSCOMPILING_EMULATOR} host/buildvm.exe"
    )
endif()

set(LUAJIT_SOURCE_PATHS)
set(LUAJIT_INSTALL_PATHS)
foreach(OUT IN LISTS LUAJIT_OUTPUT)
    list(APPEND LUAJIT_SOURCE_PATHS "<SOURCE_DIR>/src/${OUT}")
    list(APPEND LUAJIT_INSTALL_PATHS "<INSTALL_DIR>/${OUT}")
endforeach()

ExternalProject_Add(LuaJIT
    URL https://luajit.org/download/LuaJIT-2.0.4.zip
    URL_HASH SHA224=163def4ec3ad502702760c703bb81da7932573a6f4b2ff482d04ccb6
    BUILD_IN_SOURCE ON
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/LuaJIT

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${LUAJIT_BUILD}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LUAJIT_SOURCE_PATHS}
        <INSTALL_DIR>

    BUILD_BYPRODUCTS
        ${LUAJIT_INSTALL_PATHS}
)

ExternalProject_Add_Step(LuaJIT install-lua-files
    COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/src/jit <INSTALL_DIR>/lua/jit
    DEPENDEES configure
)

ExternalProject_Get_Property(LuaJIT INSTALL_DIR SOURCE_DIR)

add_library(LuaJIT_library SHARED IMPORTED)

if (lua51.lib IN_LIST LUAJIT_OUTPUT)
    set_target_properties(LuaJIT_library PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lua51.dll
        IMPORTED_IMPLIB   ${INSTALL_DIR}/lua51.lib
    )
else()
    set_target_properties(LuaJIT_library PROPERTIES
        IMPORTED_LOCATION ${INSTALL_DIR}/lua51.dll
        IMPORTED_IMPLIB   ${INSTALL_DIR}/lua51.dll
    )
endif()

# Bit of a hack, include dir won't exist yet after the generate stage but CMake
# requires it to be.
file(MAKE_DIRECTORY ${SOURCE_DIR}/src)
target_include_directories(LuaJIT_library
    INTERFACE
        ${SOURCE_DIR}/src
)

add_executable(LuaJIT_exe IMPORTED)
set_target_properties(LuaJIT_exe PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/luajit.exe
)


add_subdirectory(src)
add_subdirectory(packaging)
