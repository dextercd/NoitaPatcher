#include <iostream>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <unordered_map>

#include <MinHook.h>
#include <vs2013/init.hpp>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "calling_convention.hpp"
#include "components.hpp"
#include "damage_detail.hpp"
#include "entity_serialisation.hpp"
#include "executable_info.hpp"
#include "extended_logs.hpp"
#include "game_pause.hpp"
#include "game_mode.hpp"
#include "world_info.hpp"
#include "global_extensions.hpp"
#include "lua_util.hpp"
#include "memory_pattern.hpp"
#include "noita.hpp"
#include "physics.hpp"
#include "x86.hpp"
#include "noita_ui.hpp"
#include "crosscall.hpp"
#include "version_string.hpp"
#include "magic_nums.hpp"
#include "material.hpp"

struct FireWandInfo {
    std::uint32_t* rng;
    fire_wand_function_t func;
};

fire_wand_function_t original_fire_wand_function;

platform_shooter_damage_message_handler_t original_ps_damage_message_handler;
struct PlatformShooterDamageMessageHookCreator;

lua_State* current_lua_state;
int player_entity_id = -1;
int inject_updated_entity_id = -1;

EntityManager* entity_manager;
entity_get_by_id_t entity_get_by_id;
set_active_held_entity_t set_active_held_entity;
send_message_use_item_t use_item;
get_game_global_t get_game_global;

void* duplicate_pixel_scene_check = nullptr;

DeathMatch* g_deathmatch;
SystemManager* system_manager;

namespace np {
np::get_component_by_id_t get_component_by_id;
}

int* updated_entity_id;

void find_updated_entity_id_var(lua_State* L)
{
    const auto& noita = ThisExecutableInfo::get();

    auto lua_GetUpdatedEntityID = get_lua_c_binding(L, "GetUpdatedEntityID");
    if (!lua_GetUpdatedEntityID)
        return;

    auto pattern = make_pattern(
        Bytes{0x8b, 0x35},
        Capture{"updated_entity_id", 4},
        Bytes{0x6a, 0x01}
    );

    auto result = pattern.search(noita,
        (void*)lua_GetUpdatedEntityID,
        (char*)lua_GetUpdatedEntityID + 0x300
    );

    if (!result)
        return;

    updated_entity_id = result.get<int*>("updated_entity_id");
}


FireWandInfo find_fire_wand()
{
    FireWandInfo ret{};
    executable_info noita = ThisExecutableInfo::get();

    auto pattern = make_pattern(
        Bytes{0x80, 0xbf},
        Pad{2},
        Bytes{0x00, 0x00, 0x00, 0x0f, 0x84},
        Pad{4},
        Bytes{0x69, 0x0d},
        Capture{"rng", 4},
        Bytes{0xfd, 0x43, 0x03, 0x00}
    );

    auto result = pattern.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find fire wand function.\n";
        return ret;
    }

    auto bounds = find_function_bounds(noita, result.ptr);

    ret.rng = result.get<std::uint32_t*>("rng");
    ret.func = (fire_wand_function_t)bounds.start;

    return ret;
}

FireWandInfo fire_wand_info;

void __cdecl fire_wand_hook(
        const vec2& position,
        Entity* projectile,
        int unknown1, int unknown2, char unknown3,
        bool send_message,
        float target_x, float target_y)
{
    Entity* shooter;
    Entity* verlet_parent;
    GET_FASTCALL_REGISTER_ARGS(shooter, verlet_parent);

    auto do_callback = [&] (const char* cbname) {
        if (!current_lua_state)
            return;

        if (current_lua_state) {
            lua_getglobal(current_lua_state, "print_error");
            lua_getglobal(current_lua_state, cbname);

            // <Callback>(shooter_id:int, projectile_id:int, rng:int, position_x:number, position_y:number, target_x:number, target_y:number, send_message:boolean, unknown1:int, unknown2:int, unknown3:int)

            if (shooter)
                lua_pushinteger(current_lua_state, EntityGetId(shooter));
            else
                lua_pushnil(current_lua_state);

            if (projectile)
                lua_pushinteger(current_lua_state, EntityGetId(projectile));
            else
                lua_pushnil(current_lua_state);

            lua_pushinteger(current_lua_state, *fire_wand_info.rng);

            lua_pushnumber(current_lua_state, position.x);
            lua_pushnumber(current_lua_state, position.y);

            lua_pushnumber(current_lua_state, target_x);
            lua_pushnumber(current_lua_state, target_y);

            lua_pushboolean(current_lua_state, send_message);

            lua_pushinteger(current_lua_state, unknown1);
            lua_pushinteger(current_lua_state, unknown2);
            lua_pushinteger(current_lua_state, unknown3);

            if (lua_pcall(current_lua_state, 11, 0, -13))
                lua_pop(current_lua_state, 1); // Pop error

            lua_pop(current_lua_state, 1); // Pop error handler
        }
    };

    do_callback("OnProjectileFired");

    original_fire_wand_function(
        shooter, verlet_parent,
        position,
        projectile,
        unknown1, unknown2, unknown3,
        send_message,
        target_x, target_y
    );
    STACK_ADJUST(0x20);

    do_callback("OnProjectileFiredPost");
}

void __stdcall ps_damage_message_handler_hook(
    Entity* entity, void* unknown, void* unknown2)
{
    if (player_entity_id == -1 || EntityGetId(entity) == player_entity_id)
        original_ps_damage_message_handler(entity, unknown, unknown2);
}

struct PlatformShooterDamageMessageHookCreator {
    PlatformShooterDamageMessageHookCreator()
    {
        auto ps_damage_handler = find_ps_damage_message_handler();
        MH_CreateHook(
            (void*)ps_damage_handler,
            (void*)ps_damage_message_handler_hook,
            (void**)&original_ps_damage_message_handler
        );

        MH_EnableHook((void*)ps_damage_handler);
    }

    platform_shooter_damage_message_handler_t find_ps_damage_message_handler()
    {
        executable_info noita = ThisExecutableInfo::get();

        const std::uint8_t message_handler_bytes[] {
            0xff, 0x75, 0x10, 0x8b, 0x48, 0x0c, 0xe8,
        };

        auto mh_push = std::search(
            noita.text_start, noita.text_end,
            std::begin(message_handler_bytes), std::end(message_handler_bytes)
        );

        if (mh_push == noita.text_end) {
            std::cerr << "Couldn't find PlatformShooter damage message handler.\n";
            return nullptr;
        }

        auto func_start = std::find_end(
            noita.text_start, mh_push,
            std::begin(function_intro), std::end(function_intro)
        );

        return (platform_shooter_damage_message_handler_t)func_start;
    }
};

void find_entity_funcs()
{
    executable_info noita = ThisExecutableInfo::get();
    auto entity_pat = make_pattern(
        Bytes{0x8b, 0x0d},
        Capture{"EntityManager", 4},
        Bytes{0x83, 0xc4, 0x08, 0x50, 0xe8},
        Capture{"EntityGet", 4},
        Bytes{0x85, 0xc0, 0x74, 0xe0, 0x8b, 0xc8, 0xe8},
        Capture{"EntityTreeSetDeathState", 4},
        Bytes{0xb8, 0x01, 0x00, 0x00, 0x00}
    );
    auto entity_result = entity_pat.search(noita, noita.text_start, noita.text_end);

    if (!entity_result) {
        std::cerr << "Couldn't find entity manager funcs\n";
        return;
    }

    entity_manager = *entity_result.get<EntityManager**>("EntityManager");
    entity_get_by_id = (entity_get_by_id_t)entity_result.get_rela_call("EntityGet");

    auto str = find_rdata_string(noita, "item_physics");
    if (!str) {
        std::cerr << "Couldn't find 'item_physics' string for set_active_held_entity.\n";
        return;
    }

    auto set_active_result =
        make_pattern(Raw{str}).search(noita, noita.text_start, noita.text_end);

    if (!set_active_result) {
        std::cerr << "Couldn't find set active inventory function\n";
        return;
    }

    set_active_held_entity = (set_active_held_entity_t)std::find_end(
        noita.text_start, (std::uint8_t*)set_active_result.ptr,
        std::begin(function_intro), std::end(function_intro)
    );

    std::cout << "set_active_held_entity: " << (void*)set_active_held_entity << '\n';
}

void find_component_funcs()
{
    executable_info noita = ThisExecutableInfo::get();
    auto search_in_func =
        get_lua_c_binding(current_lua_state, "ComponentGetIsEnabled");

    auto pattern = make_pattern(
        Bytes{0xe8}, Capture{"get_component_by_id", 4},
        Bytes{0x8b, 0x08},
        Bytes{0x3b, 0x0d}, Pad{4}
    );

    auto result = pattern.search(noita, (void*)search_in_func, (char*)search_in_func + 1024);
    if (!result) {
        std::cerr << "Couldn't find get component by id func.\n";
        return;
    }

    np::get_component_by_id = (np::get_component_by_id_t)result.get_rela_call("get_component_by_id");

}

void find_deathmatch()
{
    executable_info noita = ThisExecutableInfo::get();
    auto pat = make_pattern(
        Bytes{0x33, 0xc0, 0x87, 0x06, 0x89, 0x3d},
        Capture{"g_deathmatch", 4},
        Bytes{0x8b, 0xc7, 0x8b, 0x4d, 0xf4, 0x64, 0x89, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x59, 0x5f, 0x5e, 0x8b, 0xe5, 0x5d, 0xc3}
    );

    auto result = pat.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find deathmatch struct\n";
        return;
    }

    g_deathmatch = *result.get<DeathMatch**>("g_deathmatch");
}

void find_use_item()
{
    executable_info noita = ThisExecutableInfo::get();
    auto pat = make_pattern(
        Bytes{0x89, 0x44, 0x24}, Pad{1},
        Bytes{0x8b, 0x44, 0x24}, Pad{1},
        Bytes{0x89, 0x44, 0x24}, Pad{1},
        Bytes{0xe8}, Pad{4},
        Bytes{0x8d, 0x44, 0x24}, Pad{1},
        Pad{2},
        Bytes{0xe8}, Capture{"UseItem", 4}
    );
    auto result = pat.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find UseItem\n";
        return;
    }

    use_item = (send_message_use_item_t)result.get_rela_call("UseItem");
}

void find_get_game_global()
{
    executable_info noita = ThisExecutableInfo::get();
    auto pat = make_pattern(
        Bytes{0xe8}, Capture{"GetGameGlobal", 4},
        Bytes{0x8b, 0x40, 0x48, 0x8b, 0x00, 0xc1, 0xe8, 0x02, 0xa8, 0x01}
    );
    auto result = pat.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find GetGameGlobal\n";
        return;
    }

    get_game_global = (get_game_global_t)result.get_rela_call("GetGameGlobal");
}

void find_duplicate_pixel_scene_check()
{
    auto& noita = ThisExecutableInfo::get();
    auto pattern = make_pattern(
        Bytes{0x8d, 0x56, 0x5c, 0x8d, 0x4b, 0x5c, 0xe8},
        Pad{4},
        Bytes{0x84, 0xc0, 0x0f, 0x85},
        Pad{4}
    );

    auto result = pattern.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find duplicate pixel scene check.\n";
        return;
    }

    duplicate_pixel_scene_check = (char*)result.ptr + 11;
}

void find_system_manager()
{
    auto& noita = ThisExecutableInfo::get();
    auto pattern = make_pattern(
        Bytes{0xff}, Pad{1}, Bytes{0x34, 0xb9},
        Capture{"SystemManager", 4},
        Bytes{0xff}
    );

    auto result = pattern.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        std::cerr << "Couldn't find SystemManager.\n";
        return;
    }

    system_manager = result.get<SystemManager*>("SystemManager");
}

void find_entity_serialisation()
{
    auto& noita = ThisExecutableInfo::get();
    auto ser_pattern = make_pattern(
        Bytes{0xe8}, Pad{4},
        Bytes{0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x8d, 0x4d}, Pad{1},
        Bytes{0x8b}, Pad{1},
        Bytes{0x51, 0x8b}, Pad{1},
        Bytes{0xff, 0x50, 0x10}
    );

    auto ser_result = ser_pattern.search(noita, noita.text_start, noita.text_end);
    if (!ser_result) {
        std::cerr << "Couldn't find entity serialisation routine.\n";
        return;
    }

    auto serialisation_bounds = find_function_bounds(noita, ser_result.ptr);
    np::serialise_entity_func = serialisation_bounds.start;
    std::cout << "np::serialise_entity_func: " << np::serialise_entity_func << '\n';

    auto deser_pattern = make_pattern(
        Bytes{0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x8d, 0x4d}, Pad{1},
        Bytes{0x8b}, Pad{1},
        Bytes{0x51, 0x8b}, Pad{1},
        Bytes{0xff, 0x50, 0x10}
    );

    auto deser_result = deser_pattern.search(
        noita,
        // Deserialisation function appears after serialisation
        serialisation_bounds.end,
        noita.text_end
    );

    if (!deser_result) {
        std::cerr << "Couldn't find entity deserialisation routine.\n";
        return;
    }

    auto deserialisation_bounds = find_function_bounds(noita, deser_result.ptr);
    np::deserialise_entity_func = deserialisation_bounds.start;
    std::cout << "np::deserialise_entity_func: " << np::deserialise_entity_func << '\n';
}

void find_game_mode()
{
    executable_info noita = ThisExecutableInfo::get();
    auto search_in_func =
        get_lua_c_binding(current_lua_state, "GameIsModeFullyDeterministic");

    auto pattern = make_pattern(
        Bytes{0xa1}, Capture{"game_mode_nr", 4},
        Bytes{0x8d, 0x0c, 0x40},
        Bytes{0xa1}, Capture{"game_modes_begin", 4},
        Bytes{0xc1, 0xe1, 0x06},
        Bytes{0x03, 0xc1},
        Bytes{0x83, 0x78, 0x64, 0x01}
    );

    auto result = pattern.search(noita, (void*)search_in_func, (char*)search_in_func + 0x300);
    if (!result) {
        std::cerr << "Couldn't find game mode items.\n";
        return;
    }

    np::game_mode_nr = result.get<int*>("game_mode_nr");
    np::game_modes_begin = result.get<void*>("game_modes_begin");

    std::cout << "Found game mode items: " << '\n';
    std::cout << "  game_mode_nr: " << np::game_mode_nr << '\n';
    std::cout << "  game_modes_begin: " << np::game_modes_begin << '\n';
}

struct ShootProjectileFiredHooksCreator {
    ShootProjectileFiredHooksCreator()
    {
        fire_wand_info = find_fire_wand();
        MH_CreateHook(
            (void*)fire_wand_info.func,
            (void*)fire_wand_hook,
            (void**)&original_fire_wand_function
        );

        MH_EnableHook((void*)fire_wand_info.func);
    }
};

int InstallShootProjectileFiredCallbacks(lua_State* L)
{
    static ShootProjectileFiredHooksCreator hooks;
    return 0;
}

struct DamageDetailsHooksCreators {
    DamageDetailsHooksCreators()
    {
        auto& noita = ThisExecutableInfo::get();
        auto error_string_location = find_rdata_string(
                noita,
                "TakeDamage_Impl() - DamageModelComponent couldn't be found");

        if (!error_string_location) {
            std::cerr << "Couldn't find TakeDamage_Impl() string\n";
            return;
        }

        auto pattern = make_pattern(Bytes{0x68}, Raw{error_string_location});
        auto result = pattern.search(noita, noita.text_start, noita.text_end);

        if (!result) {
            std::cerr << "Couldn't find usage of TakeDamage_Impl() string\n";
            return;
        }

        auto take_damage_impl = find_function_bounds(noita, result.ptr);
        std::cout << "found TakeDamage_Impl: " << take_damage_impl.start << '\n';
        np::install_damage_detail_hook((void*)take_damage_impl.start);

        GlobalExtensions::instance()
            .add_extension("GetDamageDetails", np::GetDamageDetails);
    }
};

int InstallDamageDetailsPatch(lua_State* L)
{
    static DamageDetailsHooksCreators hooks;
    return 0;
}

int SetProjectileSpreadRNG(lua_State* L)
{
    std::uint32_t rng_value = luaL_checkinteger(L, 1);
    *fire_wand_info.rng = rng_value;
    return 0;
}

int RegisterPlayerEntityId(lua_State* L)
{
    static PlatformShooterDamageMessageHookCreator hook;
    player_entity_id = luaL_checkinteger(L, 1);
    return 0;
}

// SetActiveHeldEntity(entity_id:int, item_id:int, unknown:bool, make_noise:bool)
int SetActiveHeldEntity(lua_State* L)
{
    int entity_id = luaL_checkinteger(L, 1);
    int item_id = luaL_checkinteger(L, 2);
    bool unknown = lua_toboolean(L, 3);
    bool make_noise = lua_toboolean(L, 4);

    auto entity = entity_get_by_id(entity_manager, entity_id);
    auto item = entity_get_by_id(entity_manager, item_id);
    if (!entity)
        luaL_error(L, "Entity %d not found.", entity_id);

    set_active_held_entity(entity, item, unknown, make_noise);
    #ifdef __GNUC__
    asm("add $0x8, %esp");
    #else
    __asm { add esp, 0x8 }
    #endif
    return 0;
}

int lua_SetPlayerEntity(lua_State* L)
{
    int entity_id = luaL_checkinteger(L, 1);
    int slot_nr = 0;

    if (lua_gettop(L) >= 2)
        slot_nr = luaL_checkinteger(L, 2);

    if (slot_nr >= g_deathmatch->player_entities.size())
        return luaL_error(L, "SetPlayerEntity(entity_id, %d) slot number is out of range", slot_nr);

    auto entity = entity_get_by_id(entity_manager, entity_id);
    g_deathmatch->player_entities[slot_nr] = entity;
    return 0;
}

int lua_GetPlayerEntity(lua_State* L)
{
    int slot_nr = 0;
    if (lua_gettop(L) >= 1)
        slot_nr = luaL_checkinteger(L, 1);

    if (slot_nr >= g_deathmatch->player_entities.size())
        return luaL_error(L, "GetPlayerEntity(%d) slot number is out of range", slot_nr);

    auto entity = g_deathmatch->player_entities[slot_nr];
    if (entity) {
        lua_pushinteger(L, EntityGetId(entity));
        return 1;
    }

    return 0;
}

// EnableGameSimulatePausing(enabled: bool)
int EnableGameSimulatePausing(lua_State* L)
{
    bool enabled = lua_toboolean(L, 1);

    executable_info noita = ThisExecutableInfo::get();
    install_game_pause_patch(noita);
    set_game_simulate_pausing_enabled(enabled);

    return 0;
}

// EnableInventoryGuiUpdate(enabled: bool)
int EnableInventoryGuiUpdate(lua_State* L)
{
    bool enabled = lua_toboolean(L, 1);
    disable_inventory_gui = !enabled;
    return 0;
}

// EnablePlayerItemPickUpper(enabled: bool)
int EnablePlayerItemPickUpper(lua_State* L)
{
    bool enabled = lua_toboolean(L, 1);
    disable_player_item_pick_upper = !enabled;
    return 0;
}

int UseItem(lua_State* L)
{
    int responsible_entity_id = luaL_checkinteger(L, 1);
    int item_entity_id = luaL_checkinteger(L, 2);
    bool ignore_reload = lua_toboolean(L, 3);
    bool charge = lua_toboolean(L, 4);
    bool started_using_this_frame = lua_toboolean(L, 5);
    float pos_x = luaL_checknumber(L, 6);
    float pos_y = luaL_checknumber(L, 7);
    float target_x = luaL_checknumber(L, 8);
    float target_y = luaL_checknumber(L, 9);

    auto item_entity = entity_get_by_id(entity_manager, item_entity_id);
    auto message = Message_UseItem{};
    message.mIgnoreReload = ignore_reload;
    message.mCharge = charge;
    message.mStartedUsingThisFrame = started_using_this_frame;
    message.mPos.x = pos_x;
    message.mPos.y = pos_y;
    message.mTarget.x = target_x;
    message.mTarget.y = target_y;

    auto restore_entity = *updated_entity_id;
    *updated_entity_id = item_entity_id;

    use_item(item_entity, &message);

    *updated_entity_id = restore_entity;

    return 0;
}

int SilenceLogs(lua_State* L)
{
    auto& noita = ThisExecutableInfo::get();

    auto logstr = luaL_checkstring(L, 1);
    auto str_location = find_rdata_string(noita, logstr);
    if (!str_location) {
        lua_pushboolean(L, false);
        return 1;
    }

    auto pattern = make_pattern(
        Bytes{0x68}, Raw{str_location},
        Bytes{0xb9}, Pad{4},
        Bytes{0xe8}, Pad{4}
    );

    auto result = pattern.search(noita, noita.text_start, noita.text_end);
    if (!result) {
        lua_pushboolean(L, false);
        return 1;
    }

    std::uint8_t patch[15] = {
        0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,       // nop 7
        0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, // nop 8
    };

    auto location = (void*)result.ptr;
    DWORD prot_restore;
    auto prot_result = VirtualProtect(
        location,
        sizeof(patch),
        PAGE_READWRITE,
        &prot_restore
    );

    if (!prot_result) {
        lua_pushboolean(L, false);
        return 1;
    }

    std::memcpy(location, patch, sizeof(patch));
    VirtualProtect(
        location,
        sizeof(patch),
        prot_restore,
        &prot_restore
    );

    lua_pushboolean(L, true);
    return 1;
}

int ForceLoadPixelScene(lua_State* L)
{
    auto lua_load_pixel_scene = get_lua_c_binding(L, "LoadPixelScene");
    if (!duplicate_pixel_scene_check)
        return lua_load_pixel_scene(L);

    std::uint8_t patch[2] = {0x31, 0xc0};

    std::uint8_t original[std::size(patch)];
    std::memcpy(original, duplicate_pixel_scene_check, sizeof(original));

    DWORD prot_restore;
    DWORD discard;

    VirtualProtect(duplicate_pixel_scene_check, sizeof(patch), PAGE_READWRITE, &prot_restore);
    std::memcpy(duplicate_pixel_scene_check, patch, sizeof(patch));
    VirtualProtect(duplicate_pixel_scene_check, sizeof(patch), prot_restore, &discard);

    auto ret = lua_load_pixel_scene(L);

    VirtualProtect(duplicate_pixel_scene_check, sizeof(original), PAGE_READWRITE, &prot_restore);
    std::memcpy(duplicate_pixel_scene_check, original, sizeof(original));
    VirtualProtect(duplicate_pixel_scene_check, sizeof(original), prot_restore, &discard);

    return ret;
}

int EnableExtendedLogging(lua_State* L)
{
    if (lua_toboolean(L, 1)) {
        auto& noita = ThisExecutableInfo::get();
        np::enable_extended_logging_hook(noita, L);
    } else {
        np::disable_extended_logging_hook();
    }

    return 0;
}

int EnableLogFiltering(lua_State* L)
{
    np::do_log_filtering = lua_toboolean(L, 1);
    return 0;
}

int EnablePCallErrorFilter(lua_State* L)
{
    np::filter_pcall_errors = lua_toboolean(L, 1);
    return 0;
}

std::unordered_map<std::string, void*> disabled_systems;

void __stdcall disable_updates(void*) {}

int ComponentUpdatesSetEnabled(lua_State* L)
{
    auto system_name_ = ulua_checkstringview(L, 1);
    bool change_to = lua_toboolean(L, 2);

    std::string system_name{system_name_};

    auto current = disabled_systems.find(system_name) == std::end(disabled_systems);

    if (current == change_to) {
        lua_pushboolean(L, true);
        return 1;
    }

    std::string search = "class " + system_name;

    for (auto&& system : system_manager->mSystems) {
        if (system->vtable->get_system_name(system).as_view() == search) {
            auto address = (void**)&system->vtable->update_components;
            void* value_to_write{};
            if (change_to) {
                value_to_write = disabled_systems[system_name];
                disabled_systems.erase(system_name);
            } else {
                value_to_write = (void*)disable_updates;
                disabled_systems[system_name] = *address;
            }

            DWORD prot_restore;
            VirtualProtect(address, sizeof(*address), PAGE_READWRITE, &prot_restore);
            *address = value_to_write;
            VirtualProtect(address, sizeof(*address), prot_restore, &prot_restore);

            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

int SetPauseState(lua_State* L)
{
    int value = luaL_checkinteger(L, 1);

    if (!get_game_global)
        luaL_error(L, "Couldn't find function for retrieving game global");

    auto GG = get_game_global();

    if (!GG->pause_state)
        luaL_error(L, "Game global is missing pause state");

    lua_pushinteger(L, *GG->pause_state);
    *GG->pause_state = value;

    return 1;
}

int GetPauseState(lua_State* L)
{
    if (!get_game_global)
        luaL_error(L, "Couldn't find function for retrieving game global");

    auto GG = get_game_global();

    if (!GG->pause_state)
        luaL_error(L, "Game global is missing pause state");

    lua_pushinteger(L, *GG->pause_state);
    return 1;
}

int GetWorldInfo(lua_State* L)
{
    auto opt_world_info = find_world_info();
    if (!opt_world_info)
        return 0;

    auto& world_info = *opt_world_info;

    lua_newtable(L);
    lua_pushinteger(L, (std::uintptr_t)world_info.get_cell);
    lua_setfield(L, -2, "get_cell");
    lua_pushinteger(L, (std::uintptr_t)world_info.remove_cell);
    lua_setfield(L, -2, "remove_cell");
    lua_pushinteger(L, (std::uintptr_t)world_info.construct_cell);
    lua_setfield(L, -2, "construct_cell");
    lua_pushinteger(L, (std::uintptr_t)world_info.chunk_loaded);
    lua_setfield(L, -2, "chunk_loaded");
    lua_pushinteger(L, (std::uintptr_t)world_info.game_global);
    lua_setfield(L, -2, "game_global");

    return 1;
}

static const luaL_Reg nplib[] = {
    {"InstallShootProjectileFiredCallbacks", InstallShootProjectileFiredCallbacks},
    {"InstallDamageDetailsPatch", InstallDamageDetailsPatch},
    {"SetProjectileSpreadRNG", SetProjectileSpreadRNG},
    {"RegisterPlayerEntityId", RegisterPlayerEntityId},
    {"SetActiveHeldEntity", SetActiveHeldEntity},
    {"SetPlayerEntity", lua_SetPlayerEntity},
    {"GetPlayerEntity", lua_GetPlayerEntity},
    {"EnableGameSimulatePausing", EnableGameSimulatePausing},
    {"EnableInventoryGuiUpdate", EnableInventoryGuiUpdate},
    {"EnablePlayerItemPickUpper", EnablePlayerItemPickUpper},
    {"UseItem", UseItem},
    {"SilenceLogs", SilenceLogs},
    {"ForceLoadPixelScene", ForceLoadPixelScene},
    {"EnableExtendedLogging", EnableExtendedLogging},
    {"EnableLogFiltering", EnableLogFiltering},
    {"EnablePCallErrorFilter", EnablePCallErrorFilter},
    {"ComponentUpdatesSetEnabled", ComponentUpdatesSetEnabled},
    {"SerializeEntity", np::SerializeEntity},
    {"DeserializeEntity", np::DeserializeEntity},
    {"PhysBodySetTransform", np::PhysBodySetTransform},
    {"PhysBodyGetTransform", np::PhysBodyGetTransform},
    {"SetGameModeDeterministic", np::SetGameModeDeterministic},
    {"SetPauseState", SetPauseState},
    {"GetPauseState", GetPauseState},
    {"GetWorldInfo", GetWorldInfo},
    {"SetInventoryCursorEnabled", np::SetInventoryCursorEnabled},
    {"CrossCallAdd", np::CrossCallAdd},
    {"CrossCall", np::CrossCall},
    {"GetVersionString", lua_GetVersionString},
    {"MagicNumbersSetValue", lua_MagicNumbersSetValue},
    {"MagicNumbersGetList", lua_MagicNumbersGetList},
    {"ReloadMaterials", lua_ReloadMaterials},
    {},
};

bool np_initialised = false;

lua_State* (*luaL_newstate_original)();

lua_State* luaL_newstate_hook()
{
    auto new_state = luaL_newstate_original();
    GlobalExtensions::instance().grant_extensions(new_state);
    return new_state;
}

int luaclose_noitapatcher(lua_State* L);

extern "C" __declspec(dllexport)
int luaopen_noitapatcher(lua_State* L)
{
    std::cout << "luaopen_noitapatcher " << L << '\n';

    // Detect module unload
    lua_newuserdata(L, 0);
    lua_newtable(L);
    lua_pushcclosure(L, luaclose_noitapatcher, 0);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, "luaclose_noitapatcher");

    current_lua_state = L;
    luaL_register(L, "noitapatcher", nplib);

    if (!np_initialised) {
        vs13::initialise();
        MH_Initialize();

        find_updated_entity_id_var(L);
        find_entity_funcs();
        find_component_funcs();
        find_game_mode();
        find_deathmatch();
        find_use_item();
        find_get_game_global();
        find_duplicate_pixel_scene_check();
        find_system_manager();
        find_entity_serialisation();

        GlobalExtensions::instance().add_extension("CrossCall", np::CrossCall);

        auto lua_lib = LoadLibraryA("lua51.dll");
        auto newstate_func = (void*)GetProcAddress(lua_lib, "luaL_newstate");
        MH_CreateHook(
            newstate_func,
            (void*)luaL_newstate_hook,
            (void**)&luaL_newstate_original
        );
        MH_EnableHook(newstate_func);

        np_initialised = true;
    }

    return 1;
}

int luaclose_noitapatcher(lua_State* L)
{
    std::cout << "luaclose_noitapatcher " << L << '\n';

    if (current_lua_state != L)
        return 0;  // Different Lua state somehow? ignore

    np::crosscall_reset();

    // The Lua state is about to go away, stop using it
    current_lua_state = nullptr;
    np::do_log_filtering = false;

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);

            // Can't really deal with being unloaded at the moment... Increase
            // the usage count to prevent it from happening.
            char filename[MAX_PATH];
            GetModuleFileNameA(hinstDLL, filename, sizeof(filename));
            LoadLibrary(filename);
            break;

        case DLL_PROCESS_DETACH:
            MH_Uninitialize();
            break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

/*

package.cpath = package.cpath .. ";./mods/NoitaPatcher/?.dll"
package.path = package.path .. ";./mods/NoitaPatcher/?.lua"
np = require("noitapatcher")

*/
