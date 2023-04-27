#include "lua_util.hpp"

lua_CFunction get_lua_c_binding(lua_State* L, const char* func_name)
{
    lua_getglobal(L, func_name);
    auto f = lua_tocfunction (L, -1);
    lua_pop(L, 1);
    return f;
}
