#ifndef NP_GAME_MODE_HPP
#define NP_GAME_MODE_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace np {

extern int* game_mode_nr;
extern void* game_modes_begin;


int lua_SetGameModeDeterministic(lua_State*);
int lua_GetGameModeNr(lua_State* L);

}

#endif // Header guard
