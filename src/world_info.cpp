#include <optional>

#include "x86.hpp"
#include "memory_pattern.hpp"
#include "executable_info.hpp"
#include "noita.hpp"
#include "world_info.hpp"

const void* func_by_bytes(const executable_info& noita, std::initializer_list<std::uint8_t> init)
{
    auto pattern = make_pattern(Bytes{init});
    auto result = pattern.search(noita, noita.text_start, noita.text_end);
    if (!result)
        return nullptr;

    auto bounds = find_function_bounds(noita, result.ptr);

    return bounds.start;
}

std::optional<WorldInfo> find_world_info()
{
    WorldInfo ret{};
    const auto& noita = ThisExecutableInfo::get();

    auto get_cell = func_by_bytes(noita, {0x2d, 0x00, 0x01, 0x00, 0x00, 0xc1, 0xe2, 0x09, 0x25, 0xff, 0x01, 0x00, 0x00, 0x03, 0xd0, 0x8b, 0x41, 0x08, 0x8b, 0x04, 0x90, 0x85, 0xc0, 0x75, 0x0b});
    auto remove_cell = func_by_bytes(noita, {0x8b, 0x06, 0x8b, 0xce, 0xff, 0x90, 0x9c, 0x00, 0x00, 0x00, 0x8b, 0x06, 0x8b, 0xce, 0x6a, 0x01, 0xff, 0x10});
    auto construct_cell = func_by_bytes(noita, {0x8b, 0x46, 0x38, 0x33, 0xc9, 0x83, 0xf8, 0x01});
    auto chunk_loaded = func_by_bytes(noita, {0x55, 0x8b, 0xec, 0x8b, 0x55, 0x0c, 0x8b, 0x45, 0x08, 0x8b, 0x49, 0x08, 0xc1, 0xfa, 0x09, 0xc1, 0xf8, 0x09, 0x81, 0xea, 0x00, 0x01, 0x00, 0x00, 0x2d, 0x00, 0x01, 0x00, 0x00, 0x81, 0xe2, 0xff, 0x01, 0x00, 0x00, 0x25, 0xff, 0x01, 0x00, 0x00, 0xc1, 0xe2, 0x09, 0x03, 0xd0, 0x33, 0xc0, 0x39, 0x04, 0x91, 0x0f, 0x95, 0xc0, 0x5d, 0xc2, 0x08, 0x00});

    auto game_global = get_game_global();

    if (get_cell && remove_cell && construct_cell && chunk_loaded && game_global) {
        return WorldInfo{
            .get_cell = get_cell,
            .remove_cell = remove_cell,
            .construct_cell = construct_cell,
            .chunk_loaded = chunk_loaded,
            .game_global = game_global,
        };
    }

    return std::nullopt;
}
