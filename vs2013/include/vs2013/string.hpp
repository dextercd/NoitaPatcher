#ifndef NP_VS13_STRING_HPP
#define NP_VS13_STRING_HPP

#include <cstdint>
#include <string_view>
#include <utility>

namespace vs13 {

struct string {
    const char* buffer;
    char sso_buffer[12];
    std::size_t size_;
    std::size_t capacity_;

    bool is_sso() const
    {
        return capacity() < 0x10;
    }

    const char* c_str() const
    {
        return data();
    }

    char* data() { return (char*)(std::as_const(*this).data()); }

    const char* data() const
    {
        if (is_sso())
            return (const char*)&buffer;

        return buffer;
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }

    std::string_view as_view() const { return {c_str(), size()}; }
};

} // vs13::

#endif // Header guard
