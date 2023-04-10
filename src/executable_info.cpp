#include "executable_info.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include <cstdint>
#include <cstring>
#include <algorithm>

executable_info get_executable_info(void* module)
{
    executable_info info;
    info.module = module;

    IMAGE_NT_HEADERS* header = ImageNtHeader(module);

    auto section_byte_start = (char*)&header->OptionalHeader + header->FileHeader.SizeOfOptionalHeader;
    auto section_begin = (IMAGE_SECTION_HEADER*)section_byte_start;
    auto section_end = section_begin + header->FileHeader.NumberOfSections;

    auto text_section = std::find_if(section_begin, section_end,
        [](IMAGE_SECTION_HEADER& section) {
            return std::strcmp((char*)section.Name, ".text") == 0;
        });
    auto rdata_section = std::find_if(section_begin, section_end,
        [](IMAGE_SECTION_HEADER& section) {
            return std::strcmp((char*)section.Name, ".rdata") == 0;
        });

    info.text_start = (std::uint8_t*)module + text_section->VirtualAddress;
    info.text_end = info.text_start + text_section->SizeOfRawData;
    info.rdata_start = (std::uint8_t*)module + rdata_section->VirtualAddress;
    info.rdata_end = info.rdata_start + rdata_section->SizeOfRawData;

    char filename[MAX_PATH];
    GetModuleFileName((HMODULE)module, filename, MAX_PATH);

    info.is_dev_build = std::strstr(filename, "noita_dev.exe") != nullptr;

    return info;
}

ThisExecutableInfo::ThisExecutableInfo()
{
    void* exe_location = GetModuleHandleA(nullptr);
    info = get_executable_info(exe_location);
}
