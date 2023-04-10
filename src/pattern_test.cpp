#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <cstdio>
#include "memory_pattern.hpp"
#include "executable_info.hpp"
#include "game_pause.hpp"

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

    executable_info info = get_executable_info(lib);

    auto pause_data = get_game_pause_data(info);

    auto adjust_ptr = [&](auto ptr) {
        return ((unsigned)ptr - (unsigned)lib + 0x400000);
    };

    std::printf("\n");

    std::printf("  pause start: %#x\n", adjust_ptr(pause_data.do_pause_update_callbacks.start));
    std::printf("  pause end  : %#x\n", adjust_ptr(pause_data.do_pause_update_callbacks.end));
    std::printf("  deathmatch start: %#x\n", adjust_ptr(pause_data.deathmatch_update.start));
    std::printf("  deathmatch end  : %#x\n", adjust_ptr(pause_data.deathmatch_update.end));
    std::printf("  call pause: %#x\n", adjust_ptr(pause_data.call_pause_addr));

    disable_game_pause(info, pause_data);

    /*auto location = pat.search(info.text_start, info.text_end);
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
    }*/
}
