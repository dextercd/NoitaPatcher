#include <iostream>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

#include <MinHook.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

const std::uint8_t function_intro[]{
    0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0
};

struct vec2 {
    float x;
    float y;
};

struct Entity {
    int EntityId;
};

struct executable_info {
    std::uint8_t* text_start;
    std::uint8_t* text_end;
};

class ThisExecutableInfo {
    executable_info info;

    ThisExecutableInfo()
    {
        void* exe_location = GetModuleHandleA(nullptr);
        IMAGE_NT_HEADERS* header = ImageNtHeader(exe_location);

        auto section_byte_start = (char*)&header->OptionalHeader + header->FileHeader.SizeOfOptionalHeader;
        auto section_begin = (IMAGE_SECTION_HEADER*)section_byte_start;
        auto section_end = section_begin + header->FileHeader.NumberOfSections;

        auto text_section = std::find_if(section_begin, section_end,
            [](IMAGE_SECTION_HEADER& section) {
                return std::strcmp((char*)section.Name, ".text") == 0;
            });

        info.text_start = (std::uint8_t*)exe_location + text_section->VirtualAddress;
        info.text_end = info.text_start + text_section->SizeOfRawData;
    }

public:
    static executable_info get()
    {
        static ThisExecutableInfo instance;
        return instance.info;
    }
};

using fire_wand_function_t = void(__fastcall*)(
    Entity* shooter, Entity* verlet_parent,
    const vec2& position,
    Entity* projectile,
    int unknown1, int unknown2, char unknown3,
    bool send_message,
    float target_x, float target_y);

fire_wand_function_t original_fire_wand_function;

struct FireWandInfo {
    int* rng;
    fire_wand_function_t func;

};

FireWandInfo find_fire_wand()
{
    FireWandInfo ret{};
    executable_info noita = ThisExecutableInfo::get();

    const std::uint8_t fire_wand_bytes[] {
        0x80, 0xbf, 0xc0, 0x02, 0x00, 0x00, 0x00, 0x0f,
        0x84, 0x1a, 0x02, 0x00, 0x00,
    };

    // Location of a cmp instruction in the fire wand function
    auto fire_wand_cmp = std::search(
        noita.text_start, noita.text_end,
        std::begin(fire_wand_bytes), std::end(fire_wand_bytes)
    );

    ret.rng = *(int**)(fire_wand_cmp + 15);
    ret.func = (fire_wand_function_t)std::find_end(
        noita.text_start, fire_wand_cmp,
        std::begin(function_intro), std::end(function_intro)
    );

    return ret;
}

FireWandInfo fire_wand_info;

void __cdecl fire_wand_hook(
        const vec2& position,
        Entity* projectile,
        int unknown1, int unknown2, char unknown3,
        bool send_message,
        float target_x, float target_y, int extra)
{
    Entity* shooter;
    Entity* verlet_parent;
    #ifdef __GNUC__
    asm("" : "=c"(shooter), "=d"(verlet_parent));
    #else
    __asm {
        mov shooter, ecx
        mov verlet_parent, edx
    }
    #endif

    std::cout << "\n\n";
    std::cout << "Shooter: " << shooter;
    if (shooter)
        std::cout << " (" << shooter->EntityId << ")";
    std::cout << '\n';

    // I can read the rng
    std::cout << "rng: " << *fire_wand_info.rng << '\n';

    // And I can change it! (Makes the wand always fire with the same spread)
    *fire_wand_info.rng = 3289471;

    original_fire_wand_function(
        shooter, verlet_parent,
        position,
        projectile,
        unknown1, unknown2, unknown3,
        send_message,
        target_x, target_y
    );
    #ifdef __GNUC__
    asm("add $0x20, %esp");
    #else
    __asm { add esp, 0x20 }
    #endif
}


extern "C" __declspec(dllexport)
void hookinstall()
{
    MH_Initialize();

    fire_wand_info = find_fire_wand();
    std::cout << "fire wand: " << (void*)fire_wand_info.func << '\n';
    MH_CreateHook(
        (void*)fire_wand_info.func,
        (void*)fire_wand_hook,
        (void**)&original_fire_wand_function
    );

    MH_EnableHook((void*)fire_wand_info.func);
}


/*

ffi = require("ffi")
ffi.cdef([[
void hookinstall();
]])
np = ffi.load("mods/noitapatcher.dll")
np.hookinstall()

*/
