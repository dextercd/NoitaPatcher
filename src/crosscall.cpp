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

void copy_value(lua_State* to, lua_State* from, int from_index)
{
    auto type = lua_type(from, from_index);
    if (type == LUA_TBOOLEAN) {
        lua_pushboolean(to, lua_toboolean(from, from_index));
    } else if (type == LUA_TNUMBER) {
        lua_pushnumber(to, lua_tonumber(from, from_index));
    } else if (type == LUA_TSTRING) {
        std::size_t length;
        auto str = lua_tolstring(from, from_index, &length);
        lua_pushlstring(to, str, length);
    } else if (type == LUA_TLIGHTUSERDATA) {
        lua_pushlightuserdata(to, lua_touserdata(from, from_index));
    } else { // Nil or unsupported type
        lua_pushnil(to);
    }
}

const int MAX_CC_RETURN = 10;

int CrossCall(lua_State* L)
{
    if (!current_lua_state)
        return luaL_error(L, "No current Lua state.");

    int restore_top = lua_gettop(current_lua_state);

    int l_top = lua_gettop(L);
    int argument_count = l_top - 1;

    if (l_top < 1 || !lua_isstring(L, 1))
        return luaL_error(L, "Expected a function name argument.");

    auto reg_iterator = registered_funcs.find(lua_tostring(L, 1));
    if (reg_iterator == std::end(registered_funcs))
        return luaL_error(L, "Requested function does not exist.");

    // Push the requested function
    int ref = reg_iterator->second;
    lua_rawgeti(current_lua_state, LUA_REGISTRYINDEX, ref);

    // Special case, not actually a cross call
    if (L == current_lua_state) {
        lua_insert(current_lua_state, 1);
        lua_call(current_lua_state, argument_count, MAX_CC_RETURN);
        return MAX_CC_RETURN;
    }

    // Copy arguments across Lua states
    lua_checkstack(current_lua_state, lua_gettop(current_lua_state) + argument_count);
    for (int i = 2; i <= l_top; ++i) {
        copy_value(current_lua_state, L, i);
    }

    int call_result = lua_pcall(current_lua_state, argument_count, MAX_CC_RETURN, 0);

    if (call_result != 0) {
        auto err = std::string{lua_tostring(current_lua_state, -1)};
        lua_settop(current_lua_state, restore_top);
        return luaL_error(L, err.c_str());
    }

    // Copy results back
    l_top += MAX_CC_RETURN;
    lua_checkstack(L, l_top);
    for (int i = -MAX_CC_RETURN; i < 0; ++i) {
        copy_value(L, current_lua_state, i);
    }

    lua_settop(current_lua_state, restore_top);
    return MAX_CC_RETURN;
}

} // namespace np
