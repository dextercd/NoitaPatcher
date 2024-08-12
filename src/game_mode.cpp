#include "game_mode.hpp"

namespace np {

int* game_mode_nr;
void* game_modes_begin;

int lua_SetGameModeDeterministic(lua_State* L)
{
    if (!game_mode_nr || !game_modes_begin)
        return 0;

    auto deterministic = (bool)lua_toboolean(L, 1);

    auto game_modes = *(char**)game_modes_begin;
    auto game_mode = game_modes + 0xc0 * *game_mode_nr;
    *(int*)(game_mode + 0x64) = deterministic ? 1 : 0;

    return 0;
}

int lua_GetGameModeNr(lua_State* L)
{
    if (!game_mode_nr)
        return luaL_error(L, "Couldn't find game mode number address.");

    lua_pushinteger(L, *game_mode_nr);
    return 1;
}

}
