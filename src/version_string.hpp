#ifndef NP_VERSION_STRING_HPP
#define NP_VERSION_STRING_HPP

extern "C" {
#include <lua.h>
}

int lua_GetVersionString(lua_State* L);

#endif
