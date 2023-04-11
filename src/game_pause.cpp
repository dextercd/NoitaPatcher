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
    auto callback_str = load_address(exe,
        find_rdata_string(exe, "Mods_OnPausePreUpdate"));

    if (!callback_str) {
        callback_str = load_address(exe,
            find_rdata_string(exe, "OnPausePreUpdate"));
    }

    // The function that uses this string is responsible for invoking the
    // corresponding callbacks.
    auto pattern = make_pattern(Raw{callback_str});
    auto search_result = pattern.search(exe, exe.text_start, exe.text_end);
    auto callback_func = find_function_bounds(exe, search_result.ptr);

    // The DeathmatchUpdate function calls this when the game is paused.
    auto call_on_pause_pattern = make_pattern(
        Call{load_address(exe, callback_func.start)}
    );

    auto call_result = call_on_pause_pattern.search(exe, exe.text_start, exe.text_end);
    auto deathmatch = find_function_bounds(exe, call_result.ptr);

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
        .do_pause_update_callbacks = callback_func,
        .call_pause_addr = call_result.ptr,
        .deathmatch_update = deathmatch,
        .update_inventory_gui = find_function_bounds(exe, inventory_gui_find.ptr),
        .update_item_pick_upper = find_function_bounds(exe, item_pick_upper_find.ptr),
    };

    return ret;
}

void disable_game_pause(const executable_info& exe, const GamePauseData& game_pause)
{
    static bool game_pause_enabled = true;
    if (!game_pause_enabled)
        return;

    game_pause_enabled = false;

    make_inventory_gui_updates_hook(game_pause);
    make_item_pick_upper_updates_hook(game_pause);

    auto deathmatch_asm = disassemble(exe, game_pause.deathmatch_update);
    auto pause_start = deathmatch_asm.at_loadaddr(load_address(exe, game_pause.call_pause_addr));
    auto jump_over_pause = pause_start - 1;

    if (jump_over_pause->inst.mnemonic != ZYDIS_MNEMONIC_JLE)
        return;

    ZyanU64 simulate_start{};
    ZydisCalcAbsoluteAddress(
        &jump_over_pause->inst,
        deathmatch_asm.get_operands(jump_over_pause),
        jump_over_pause->load_location,
        &simulate_start
    );

    const auto pause_end = deathmatch_asm.at_loadaddr(simulate_start);
    for (auto it = pause_start; it != pause_end; ++it) {
        if (it->inst.mnemonic != ZYDIS_MNEMONIC_JMP)
            continue;

        ZyanU64 jump_target{};
        ZydisCalcAbsoluteAddress(
            &it->inst,
            deathmatch_asm.get_operands(it),
            it->load_location,
            &jump_target
        );

        if (jump_target < pause_end->load_location)
            continue;

        ZydisEncoderRequest req{};
        req.mnemonic = ZYDIS_MNEMONIC_JMP;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_COMPAT_32;
        req.operand_count = 1;
        req.operands[0].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[0].imm.u = simulate_start;

        void* const address = real_address(exe, it->load_location);
        DWORD restore_prot;
        VirtualProtect(address, it->inst.length, PAGE_READWRITE, &restore_prot);

        ZyanUSize instruction_length = it->inst.length;
        auto encode_status = ZydisEncoderEncodeInstructionAbsolute(
                &req, address, &instruction_length, it->load_location);

        VirtualProtect(address, it->inst.length, restore_prot, &restore_prot);
    };
}
