#ifndef NP_EXECUTABLE_INFO_HPP
#define NP_EXECUTABLE_INFO_HPP

#include <cstdint>

struct executable_info {
    void* module;

    std::uint8_t* text_start;
    std::uint8_t* text_end;

    std::uint8_t* rdata_start;
    std::uint8_t* rdata_end;

    void* import_descriptors;

    bool is_dev_build = false;
};

executable_info get_executable_info(void* module);

class ThisExecutableInfo {
    executable_info info;

    ThisExecutableInfo();

public:
    static const executable_info& get()
    {
        static ThisExecutableInfo instance;
        return instance.info;
    }
};

void** iat_address(const executable_info& exe, const char* executable_name, const char* function_name);

#endif // Header guard
