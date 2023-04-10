#include <algorithm>

#include "memory_pattern.hpp"
#include "executable_info.hpp"
#include "x86.hpp"

struct BytesCheck : PatternCheck {
    std::vector<char> bytes;

    BytesCheck(std::vector<char> bs)
        : bytes{std::move(bs)}
    {
    }

    const void* search(const void* begin, const void* end) override
    {
        auto it = std::search(
            (const char*)begin, (const char*)end,
            std::begin(bytes), std::end(bytes)
        );

        if (it == end)
            return nullptr;

        return it;
    }

    bool check(const PatternCheckArgs& at) override
    {
        return std::equal(std::begin(bytes), std::end(bytes), (const char*)at.location);
    }
};

struct RelativeCheck : PatternCheck {
    int offset_from;
    std::uint32_t target;

    RelativeCheck(int offset, std::uint32_t tgt)
        : offset_from{offset}
        , target{tgt}
    {
    }

    const void* search(const void* begin, const void* end) override
    {
        return nullptr; // Not implemented
    }

    bool check(const PatternCheckArgs& at) override
    {
        auto where = at.load_location + offset_from;
        std::int32_t value;
        std::memcpy(&value, at.location, sizeof(value));
        return where + value == target;
    }
};

void* FindResult::capture_ptr(std::string_view capture_name)
{
    if (!*this)
        return nullptr;

    auto capture_it = std::find_if(std::begin(captures), std::end(captures),
        [&](const auto& c) { return c.name == capture_name; });

    if (capture_it == std::end(captures))
        return nullptr;

    return (char*)ptr + capture_it->offset;
}

Pattern::~Pattern() = default;

void Pattern::add(Bytes b)
{
    auto size = b.data.size();
    parts.emplace_back(
        cursor,
        size,
        std::make_shared<BytesCheck>(std::move(b.data))
    );

    cursor += size;
}

void Pattern::add(Call call)
{
    add(Bytes{0xe8});
    parts.emplace_back(
        cursor,
        sizeof(call.target),
        std::make_shared<RelativeCheck>(4, call.target)
    );
    cursor += sizeof(call.target);
}

FindResult Pattern::search(const executable_info& exe, const void* begin_, const void* end_)
{
    auto begin = (char*)begin_;
    auto end = (char*)end_;

    if (end - begin < size())
        return nullptr;

    const auto& first_part = parts.front();

    // If we find the first part in this range then there's always enough
    // space for the following parts.
    auto first_part_min = begin + first_part.offset;
    auto first_part_max = end - following_size();

    const char* first_part_loc = first_part_min;
    while (true) {
        first_part_loc = (const char*)first_part.check->search(first_part_loc, first_part_max);

        if (first_part_loc == nullptr)
            return nullptr;

        auto candidate = first_part_loc - first_part.offset;
        auto all_match =
            std::all_of(
                parts.begin() + 1, parts.end(),
                [&] (const auto& part) {
                    auto location = candidate + part.offset;
                    PatternCheckArgs args{
                        location,
                        load_address(exe, location),
                    };
                    return part.check->check(args);
                }
            );

        if (all_match)
            return make_result(candidate);

        ++first_part_loc;
    }
}

FindResult Pattern::make_result(const void* ptr)
{
    return FindResult{ptr, captures};
}
