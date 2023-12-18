#ifndef NP_WORLD_INFO_HPP
#define NP_WORLD_INFO_HPP

#include <optional>

struct WorldInfo {
    const void* get_cell;
    const void* remove_cell;
    const void* construct_cell;
    const void* chunk_loaded;
    const void* game_global;
};

std::optional<WorldInfo> find_world_info();

#endif // Header guard
