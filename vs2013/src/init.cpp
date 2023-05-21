#include <cstdlib>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vs2013/init.hpp>
#include <vs2013/memory.hpp>

namespace vs13 {

void initialise()
{
    auto runtime_library = LoadLibrary("MSVCR120.DLL");
    if (!runtime_library) {
        std::cerr << "Couldn't load vs2013 runtime library.\n";
        std::_Exit(1);
    }

    vs13::operator_new = (vs13::operator_new_signature)GetProcAddress(runtime_library, "??2@YAPAXI@Z");
    vs13::operator_delete = (vs13::operator_delete_signature)GetProcAddress(runtime_library, "??3@YAXPAX@Z");
}

};
