#ifndef NP_GAME_MODE_HPP
#define NP_GAME_MODE_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <vs2013/string.hpp>
#include <vs2013/vector.hpp>

namespace np {

struct game_mode {
    char padding[0x64];
    int deterministic;
    char padding2[0x10];
    vs13::string name;
    char padding3[48];
};

static_assert(offsetof(game_mode, deterministic) == 0x64);
static_assert(offsetof(game_mode, name) == 0x78);
static_assert(sizeof(game_mode) == 0xc0);

extern int* game_mode_nr;
extern vs13::vector<game_mode>* game_modes_vec;


int lua_SetGameModeDeterministic(lua_State*);
int lua_GetGameModeNr(lua_State* L);
int lua_GetGameModeCount(lua_State* L);
int lua_GetGameModeName(lua_State* L);

}

#endif // Header guard
