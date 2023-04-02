#include <algorithm>

#include "memory_pattern.hpp"

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

FindResult Pattern::search(void* begin_, void* end_)
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

    char* first_part_loc = first_part_min;
    while (true) {
        first_part_loc = std::search(
            first_part_loc, first_part_max,
            std::begin(first_part.bytes), std::end(first_part.bytes)
        );

        if (first_part_loc == first_part_max)
            return nullptr;

        auto candidate = first_part_loc - first_part.offset;
        auto all_match =
            std::all_of(
                parts.begin() + 1, parts.end(),
                [&] (const auto& part) {
                    return std::equal(
                        std::begin(part.bytes), std::end(part.bytes),
                        candidate + part.offset
                    );
                }
            );

        if (all_match)
            return make_result(candidate);

        ++first_part_loc;
    }
}

FindResult Pattern::make_result(void* ptr)
{
    return FindResult{ptr, captures};
}
