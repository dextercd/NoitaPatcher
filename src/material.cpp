#include <iostream>
#include <optional>

#include "memory_pattern.hpp"
#include "x86.hpp"
#include "noita.hpp"

extern "C" {
#include <lua.h>
}

namespace {

using clear_materials_t = void(__thiscall*)(void*);
using load_materials_t = void(__thiscall*)(void*, char);

struct CellDataLoadingDb {
    clear_materials_t clear_materials;
    load_materials_t load_materials;
};

std::optional<CellDataLoadingDb> make_cell_data_loading_db()
{
    auto& noita = ThisExecutableInfo::get();

    auto clear_mats_pattern = make_pattern(
        Bytes{0xc7, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00,
              0x8b, 0x86, 0x98, 0x00, 0x00, 0x00,
              0x89, 0x86, 0x9c, 0x00, 0x00, 0x00,
              0x8b, 0x86, 0xa8, 0x00, 0x00, 0x00,
              0x89, 0x86, 0xac, 0x00, 0x00, 0x00}
    );

    auto clear_mats_res = clear_mats_pattern.search(noita, noita.text_start, noita.text_end);
    if (!clear_mats_res) {
        std::cerr << "Couldn't find clear mats function.\n";
        return std::nullopt;
    }

    auto clear_mats_func_bounds = find_function_bounds(noita, clear_mats_res.ptr);

    auto log_str = find_rdata_string(noita, "Loading textures for materials...");
    auto load_mats_pattern = make_pattern(Raw{log_str});
    auto load_mats_res = load_mats_pattern.search(noita, noita.text_start, noita.text_end);
    if (!load_mats_res) {
        std::cerr << "Couldn't find load mats function.\n";
        return std::nullopt;
    }
    auto load_mats_func_bounds = find_function_bounds(noita, load_mats_res.ptr);

    std::cerr << "ClearMats: " << clear_mats_func_bounds.start << '\n';
    std::cerr << "LoadMats: " << load_mats_func_bounds.start << '\n';

    return CellDataLoadingDb{
        .clear_materials = (clear_materials_t)clear_mats_func_bounds.start,
        .load_materials = (load_materials_t)load_mats_func_bounds.start,
    };
}

CellDataLoadingDb* get_cell_data_loading_db()
{
    static auto db = make_cell_data_loading_db();
    if (db) {
        return &db.value();
    }

    return nullptr;
}

};

int lua_ReloadMaterials(lua_State* L)
{
    auto cell_data_loading_db = get_cell_data_loading_db();
    if (!cell_data_loading_db) {
        std::cerr << "Couldn't find cell data (un)loading functions.\n";
    }

    if (!get_game_global) {
        std::cerr << "Couldn't find get game global function.\n";
    }
    auto GG = get_game_global();

    cell_data_loading_db->clear_materials(GG->mCellFactory);
    cell_data_loading_db->load_materials(GG->mCellFactory, true);

    return 0;
}

