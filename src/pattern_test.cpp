#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <cstdio>
#include "memory_pattern.hpp"
#include "executable_info.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Wrong invocation!\n";
        std::cerr << "Usage:\n";
        std::cerr << "  noitadis path/to/noita.exe\n";
        return 1;
    }

    auto lib = LoadLibraryA(argv[1]);
    if (!lib) {
        std::cerr << "Couldn't load library\n";
        return 2;
    }

    auto pat = make_pattern(
        Bytes{0x33, 0xc0, 0x87, 0x06, 0x89, 0x3d},
        Capture{"g_deathmatch", 4},
        Bytes{0x8b, 0xc7, 0x8b, 0x4d, 0xf4, 0x64, 0x89, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x59, 0x5f, 0x5e, 0x8b, 0xe5, 0x5d, 0xc3}
    );

    auto adjust_ptr = [&](auto ptr) {
        return ((unsigned)ptr - (unsigned)lib + 0x400000);
    };

    executable_info info = get_executable_info(lib);
    auto location = pat.search(info.text_start, info.text_end);
    if (!location) {
        std::cout << "Not found.\n";
    } else {
        unsigned exe_location = adjust_ptr(location.ptr);
        std::printf("%#x\n", exe_location);

        for (const auto& capture : location.captures) {
            auto value = location.get<unsigned>(capture.name);
            std::printf("  %#010x (relafunc %#010x) %s\n",
                value,
                adjust_ptr(location.get_rela_call(capture.name)),
                capture.name.c_str()
            );
        }
    }
}
