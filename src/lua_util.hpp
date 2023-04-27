#ifndef NP_LUA_UTIL_HPP
#define NP_LUA_UTIL_HPP

extern "C" {
#include <lua.h>
}

lua_CFunction get_lua_c_binding(lua_State* L, const char* func_name);

#endif // Header guard
