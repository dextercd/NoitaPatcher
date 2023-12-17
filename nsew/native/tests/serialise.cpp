#include <cstdint>
#include <iostream>
#include <limits>
#include <random>

#include <nsew/serialise.hpp>

using engine = std::mt19937;

template<class T>
struct serialisation_test {
    using ser = nsew::serialise<T>;
    using limits = std::numeric_limits<T>;

    const char* name;

    serialisation_test(const char* n)
        : name{n}
    {
    }

    bool test_single(T value)
    {
        char buffer[ser::size];
        ser::write(buffer, value);
        auto const back = ser::read(buffer);

        if (value != back) {
            std::cerr << name << ": "
                      << "read(write(" << +value << ")) == " << +back << '\n';

            return false;
        }

        return true;
    }

    bool edge_cases()
    {
        T cases[]{(T)0, (T)-1, limits::min(), limits::max()};

        for (auto c : cases) {
            if (!test_single(c))
                return false;
        }

        return true;
    }

    bool random()
    {
        auto rand = engine{};
        auto dist = std::uniform_int_distribution{
            +limits::min(),
            +limits::max(),
        };

        for (int i = 0; i != 100000; ++i) {
            auto const value = dist(rand);
            if (!test_single(value))
                return false;
        }

        return true;
    }
};

template<class T>
bool test_for(const char* name)
{
    serialisation_test<T> test{name};
    return test.edge_cases() && test.random();
}

#define TEST_FOR(T) (test_for<T>(#T))

int main()
{
    return !(
        TEST_FOR(char) && TEST_FOR(signed char) && TEST_FOR(unsigned char) &&
        TEST_FOR(std::int16_t) && TEST_FOR(std::uint16_t) && TEST_FOR(std::int32_t) &&
        TEST_FOR(std::uint32_t) && TEST_FOR(std::int64_t) && TEST_FOR(std::uint64_t));
}
