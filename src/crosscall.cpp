#include <unordered_map>
#include <string>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

extern lua_State* current_lua_state;

namespace np {

std::unordered_map<std::string, int> registered_funcs;

void crosscall_reset()
{
    registered_funcs.clear();
}

int CrossCallAdd(lua_State* L)
{
    if (L != current_lua_state)
        return luaL_error(L, "Unexpected Lua state used.");

    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_isfunction(L, 2))
        return luaL_error(L, "Expected function name string and one function argument.");

    int ref = -1;

    auto name = std::string{lua_tostring(L, 1)};
    auto f = registered_funcs.find(name);
    if (f != std::end(registered_funcs)) {
        ref = f->second;
        lua_rawseti(L, LUA_REGISTRYINDEX, ref);
    } else {
        ref = luaL_ref(L, LUA_REGISTRYINDEX);
        registered_funcs.emplace(std::move(name), ref);
    }

    return 0;
}

int CrossCall(lua_State* L)
{
    if (!current_lua_state)
        return luaL_error(L, "No current Lua state.");

    if (lua_gettop(L) != 1 || !lua_isstring(L, 1))
        return luaL_error(L, "Expected single function name argument.");

    auto f = registered_funcs.find(lua_tostring(L, 1));
    if (f == std::end(registered_funcs))
        return luaL_error(L, "Requested function does not exist.");

    auto ref = f->second;

    auto current_top = lua_gettop(current_lua_state);
    lua_rawgeti(current_lua_state, LUA_REGISTRYINDEX, ref);
    auto call_result = lua_pcall(current_lua_state, 0, 1, 0);

    if (call_result != 0) {
        auto err = std::string{lua_tostring(current_lua_state, -1)};
        lua_settop(current_lua_state, current_top);
        return luaL_error(L, lua_tostring(current_lua_state, 1));
    }

    auto has_return_value = false;
    std::string return_value;

    if (!lua_isnil(current_lua_state, -1)) {
        has_return_value = true;
        return_value = lua_tostring(current_lua_state, -1);
    }

    // Careful stack handling in case current_lua_state and L are the same
    lua_settop(current_lua_state, current_top);

    auto return_count = 0;
    if (has_return_value) {
        lua_pushstring(L, return_value.c_str());
        ++return_count;
    }

    return return_count;
}

} // namespace np
