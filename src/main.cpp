#include <iostream>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

#include <MinHook.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "executable_info.hpp"
#include "noita.hpp"
#include "x86.hpp"

struct FireWandInfo {
    std::uint32_t* rng;
    fire_wand_function_t func;
};

fire_wand_function_t original_fire_wand_function;
platform_shooter_damage_message_handler_t original_ps_damage_message_handler;
lua_State* current_lua_state;
int player_entity_id = -1;

FireWandInfo find_fire_wand()
{
    FireWandInfo ret{};
    executable_info noita = ThisExecutableInfo::get();

    const std::uint8_t fire_wand_bytes[] {
        0x80, 0xbf, 0xc0, 0x02, 0x00, 0x00, 0x00, 0x0f, 0x84,
    };

    const std::uint8_t fire_wand_bytes_dev[] {
        0x80, 0xbf, 0xc8, 0x02, 0x00, 0x00, 0x00, 0x0f, 0x84,
    };

    // Location of a cmp instruction in the fire wand function
    auto fire_wand_cmp = std::search(
        noita.text_start, noita.text_end,
        std::begin(fire_wand_bytes), std::end(fire_wand_bytes)
    );

    if (fire_wand_cmp == noita.text_end) {
        fire_wand_cmp = std::search(
            noita.text_start, noita.text_end,
            std::begin(fire_wand_bytes_dev), std::end(fire_wand_bytes_dev)
        );

        if (fire_wand_cmp == noita.text_end) {
            std::cerr << "Couldn't find fire wand function.\n";
            return ret;
        }
    }

    ret.rng = *(std::uint32_t**)(fire_wand_cmp + 15);
    ret.func = (fire_wand_function_t)std::find_end(
        noita.text_start, fire_wand_cmp,
        std::begin(function_intro), std::end(function_intro)
    );

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
    #ifdef __GNUC__
    asm("" : "=c"(shooter), "=d"(verlet_parent));
    #else
    __asm {
        mov shooter, ecx
        mov verlet_parent, edx
    }
    #endif

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

    do_callback("OnProjectileFiredPost");

    #ifdef __GNUC__
    asm("add $0x20, %esp");
    #else
    __asm { add esp, 0x20 }
    #endif
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

void __stdcall ps_damage_message_handler_hook(
    Entity* entity, void* unknown, void* unknown2
)
{
    if (player_entity_id == -1 || EntityGetId(entity) == player_entity_id)
        original_ps_damage_message_handler(entity, unknown, unknown2);
}

extern "C" __declspec(dllexport)
void install_hooks()
{
    MH_Initialize();

    fire_wand_info = find_fire_wand();
    std::cout << "fire wand: " << (void*)fire_wand_info.func << '\n';
    MH_CreateHook(
        (void*)fire_wand_info.func,
        (void*)fire_wand_hook,
        (void**)&original_fire_wand_function
    );

    auto ps_damage_handler = find_ps_damage_message_handler();
    MH_CreateHook(
        (void*)ps_damage_handler,
        (void*)ps_damage_message_handler_hook,
        (void**)&original_ps_damage_message_handler
    );

    MH_EnableHook((void*)fire_wand_info.func);
    MH_EnableHook((void*)ps_damage_handler);
}

int SetProjectileSpreadRNG(lua_State* L)
{
    std::uint32_t rng_value = luaL_checkinteger(L, 1);
    *fire_wand_info.rng = rng_value;
    return 0;
}

int RegisterPlayerEntityId(lua_State* L)
{
    player_entity_id = luaL_checkinteger(L, 1);
    return 0;
}

int luaclose_noitapatcher(lua_State* L);

static const luaL_Reg nplib[] = {
    {"SetProjectileSpreadRNG", SetProjectileSpreadRNG},
    {"RegisterPlayerEntityId", RegisterPlayerEntityId},
    {},
};

bool np_initialised = false;

extern "C" __declspec(dllexport)
int luaopen_noitapatcher(lua_State* L)
{
    std::cout << "luaopen_noitapatcher" << L << '\n';

    current_lua_state = L;
    luaL_register(L, "noitapatcher", nplib);

    // Detect module unload
    lua_newuserdata(L, 0);
    lua_newtable(L);
    lua_pushcclosure(L, luaclose_noitapatcher, 0);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, "luaclose_noitapatcher");

    if (!np_initialised) {
        install_hooks();
        np_initialised = true;
    }

    return 1;
}

int luaclose_noitapatcher(lua_State* L)
{
    std::cout << "luaclose_noitapatcher" << L << '\n';

    if (current_lua_state != L)
        return 0;  // Different Lua state somehow? ignore

    // The Lua state is about to go away, stop using it
    current_lua_state = nullptr;

    return 0;
}

/*

package.cpath = package.cpath .. ";./mods/?.dll"
np = require("noitapatcher")

function OnProjectileFired(shooter_id, projectile_id, rng, position_x, position_y, target_x, target_y, send_message, unknown1, unknown2, unknown3)
    print(entity)
    print(rng)

    np.SetProjectileSpreadRNG(0)
end

*/
