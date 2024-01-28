#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Zydis/Utils.h>
#include <Zydis/Encoder.h>
#include <MinHook.h>

#include "x86.hpp"
#include "memory_pattern.hpp"
#include "game_pause.hpp"
#include "executable_info.hpp"
#include "noita.hpp"
#include "utils.hpp"

extern int player_entity_id;

bool disable_inventory_gui = false;
bool disable_player_item_pick_upper = false;

namespace {

update_component_t update_inventory_gui;
update_component_t update_pick_upper;

struct GamePauseSystemHooks {
    void __thiscall InventoryGuiUpdate(Entity* entity, void* component) {
        if (disable_inventory_gui)
            return;

        return update_inventory_gui(this, entity, component);
    }

    void __thiscall ItemPickUpperUpdate(Entity* entity, void* component) {
        if (disable_player_item_pick_upper && EntityGetId(entity) == player_entity_id)
            return;

        return update_pick_upper(this, entity, component);
    }
};

void make_inventory_gui_updates_hook(const GamePauseData& game_pause)
{
    auto inventory_gui_target = (void*)game_pause.update_inventory_gui.start;
    MH_CreateHook(
        inventory_gui_target,
        memfn_voidp(&GamePauseSystemHooks::InventoryGuiUpdate),
        (void**)&update_inventory_gui);
    MH_EnableHook(inventory_gui_target);
}

void make_item_pick_upper_updates_hook(const GamePauseData& game_pause)
{
    auto item_pick_upper_target = (void*)game_pause.update_item_pick_upper.start;
    MH_CreateHook(
        item_pick_upper_target,
        memfn_voidp(&GamePauseSystemHooks::ItemPickUpperUpdate),
        (void**)&update_pick_upper);
    MH_EnableHook(item_pick_upper_target);
}

}

GamePauseData get_game_pause_data(const executable_info& exe)
{
    auto callback_str = find_rdata_string(exe, "OnPausePreUpdate");
    auto call_pause_pattern = make_pattern(Bytes{0xba}, Raw{callback_str});
    auto call_pause_result = call_pause_pattern.search(exe, exe.text_start, exe.text_end);

    auto deathmatch_str = find_rdata_string(exe, "ending_no_game_over_menu");

    auto pattern = make_pattern(Raw{deathmatch_str});
    auto search_result = pattern.search(exe, exe.text_start, exe.text_end);
    auto deathmatch = find_function_bounds(exe, search_result.ptr);
    // Fixup wrong function end
    deathmatch.end = (make_pattern(Bytes{0xc2, 0x04, 0x00})).search(exe, deathmatch.start, exe.text_end).ptr;

    auto inventory_gui_pattern = make_pattern(
        Bytes({
            0xc7, 0x44, 0x24, 0x08, 0x00, 0x00, 0x80, 0x3f, 0xc7, 0x44, 0x24,
            0x04, 0x00, 0x00, 0x7a, 0x44, 0xc7, 0x04, 0x24, 0x00, 0x00, 0x80,
            0x3f,
        }));
    auto inventory_gui_find = inventory_gui_pattern.search(exe, exe.text_start, exe.text_end);

    auto item_pick_upper_pattern = make_pattern(
        Bytes({
            0xf3, 0x0f, 0x10, 0x4b, 0x50, 0xf3, 0x0f, 0x10, 0x43, 0x54, 0xf3,
            0x0f, 0x58, 0xca,
        }));
    auto item_pick_upper_find = item_pick_upper_pattern.search(exe, exe.text_start, exe.text_end);
    std::cout << "item_pick_upper_find: " << item_pick_upper_find.ptr << '\n';

    GamePauseData ret{
        .pause_callback_str = callback_str,
        .call_pause_addr = call_pause_result.ptr,
        .deathmatch_update = deathmatch,
        .update_inventory_gui = find_function_bounds(exe, inventory_gui_find.ptr),
        .update_item_pick_upper = find_function_bounds(exe, item_pick_upper_find.ptr),
    };

    std::cout << (void*)ret.pause_callback_str << '\n';
    std::cout << (void*)ret.call_pause_addr << '\n';
    std::cout << (void*)ret.deathmatch_update.start << ", " << ret.deathmatch_update.end << '\n';

    return ret;
}

void __thiscall (*OriginalGameSimulate)(void* this_, float dt);

bool simulate_pausing_enabled = true;

void __thiscall PatchedGameSimulate(void* this_, float dt)
{
    OriginalGameSimulate(this_, dt);

    auto GG = get_game_global();

    if (!simulate_pausing_enabled) {
        auto pause_state = *GG->pause_state;

        if (pause_state > 0) {
            *GG->pause_state = 0;
            OriginalGameSimulate(this_, dt);
            *GG->pause_state = pause_state;
        }
    }
}

void install_game_pause_patch(const executable_info& exe)
{
    static bool patch_installed = false;
    if (patch_installed)
        return;

    patch_installed = true;

    auto game_pause = get_game_pause_data(exe);

    make_inventory_gui_updates_hook(game_pause);
    make_item_pick_upper_updates_hook(game_pause);

    MH_CreateHook(
        (void*)game_pause.deathmatch_update.start,
        (void*)PatchedGameSimulate,
        (void**)&OriginalGameSimulate
    );

    MH_EnableHook((void*)game_pause.deathmatch_update.start);
}

void set_game_simulate_pausing_enabled(bool allow)
{
    simulate_pausing_enabled = allow;
}
