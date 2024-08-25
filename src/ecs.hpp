#ifndef NP_ECS_HPP
#define NP_ECS_HPP

extern "C" {
#include <lua.h>
}

namespace np {
int lua_GetComponentAddress(lua_State* L);
int lua_GetEntityAddress(lua_State* L);
}

#endif // Header guard
