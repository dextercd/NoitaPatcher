#include "game_mode.hpp"

namespace np {

int* game_mode_nr;
vs13::vector<game_mode>* game_modes_vec;

int lua_SetGameModeDeterministic(lua_State* L)
{
    if (!game_mode_nr || !game_modes_vec)
        return 0;

    auto deterministic = (bool)lua_toboolean(L, 1);

    (*game_modes_vec)[*game_mode_nr].deterministic = deterministic;

    return 0;
}

int lua_GetGameModeNr(lua_State* L)
{
    if (!game_mode_nr)
        return luaL_error(L, "Couldn't find game mode number address.");

    lua_pushinteger(L, *game_mode_nr);
    return 1;
}

int lua_GetGameModeCount(lua_State* L)
{
    lua_pushinteger(L, game_modes_vec->size());
    return 1;
}

int lua_GetGameModeName(lua_State* L)
{
    int nr{};
    if (lua_gettop(L) == 0) {
        nr = *game_mode_nr;
    } else {
        nr = luaL_checkinteger(L, 1);
    }

    if (nr < 0 || nr >= game_modes_vec->size()) {
        return luaL_error(L, "Game mode number out of range");
    }

    lua_pushstring(L, (*game_modes_vec)[nr].name.c_str());
    return 1;
}

}
