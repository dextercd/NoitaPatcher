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

    auto impdes_offset = header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    info.import_descriptors = (char*)module + impdes_offset;

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

void** iat_address(
        const executable_info& exe,
        const char* executable_name,
        const char* function_name)
{
    auto base = (const char*)exe.module;
    auto id = (IMAGE_IMPORT_DESCRIPTOR*)exe.import_descriptors;
    for (; id->OriginalFirstThunk; ++id) {
        auto import_name = (const char*)(base + id->Name);
        if (std::strcmp(executable_name, import_name) != 0)
            continue;

        auto original_thunk = (IMAGE_THUNK_DATA*)(base + id->OriginalFirstThunk);
        auto thunk = (IMAGE_THUNK_DATA*)(base + id->FirstThunk);

        while (original_thunk) {
            auto import_by_name = (IMAGE_IMPORT_BY_NAME*)(base + original_thunk->u1.AddressOfData);
            auto import_func_name = (const char*)&import_by_name->Name;

            if (std::strcmp(function_name, import_func_name) == 0) {
                return (void**)&thunk->u1.Function;
            }

            ++original_thunk;
            ++thunk;
        }
    }

    return nullptr;
}
