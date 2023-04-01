#ifndef NP_EXECUTABLE_INFO_HPP
#define NP_EXECUTABLE_INFO_HPP

#include <cstdint>

struct executable_info {
    std::uint8_t* text_start;
    std::uint8_t* text_end;
    bool is_dev_build = false;
};

executable_info get_executable_info(void* module);

class ThisExecutableInfo {
    executable_info info;

    ThisExecutableInfo();

public:
    static executable_info get()
    {
        static ThisExecutableInfo instance;
        return instance.info;
    }
};

#endif // Header guard
