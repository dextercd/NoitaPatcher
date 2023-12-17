#ifndef NSEW_SERIALISE_HPP
#define NSEW_SERIALISE_HPP

#include <climits>
#include <concepts>
#include <type_traits>

namespace nsew {

template<class T>
struct serialise;

template<std::unsigned_integral N>
struct serialise<N> {
    static constexpr auto size = sizeof(N);

    static constexpr void write(char* buffer, N value)
    {
        for (int i = 0; i != size; ++i) {
            buffer[i] = (value >> i * CHAR_BIT) & UCHAR_MAX;
        }
    }

    static constexpr N read(char const* buffer)
    {
        N result{};
        for (int i = 0; i != size; ++i) {
            result |= (N)(unsigned char)buffer[i] << i * CHAR_BIT;
        }
        return result;
    }
};

template<std::signed_integral N>
struct serialise<N> {
    using underlying_serialise = serialise<std::make_unsigned_t<N>>;

    static constexpr auto size = sizeof(N);

    static constexpr void write(char* buffer, N value)
    {
        return underlying_serialise::write(buffer, value);
    }

    static constexpr N read(char const* buffer)
    {
        return underlying_serialise::read(buffer);
    }
};

} // namespace nsew

#endif // NSEW_SERIALISE_HPP
