#include "lua_util.hpp"

extern "C" {
#include <lualib.h>
}

#include "extended_logs.hpp"
#include <MinHook.h>
#include <string>

extern lua_State* current_lua_state;

namespace {

lua_CFunction print_func;
lua_CFunction original_print_func;

bool filter_log(const char* source, int linenumber)
{
    lua_getglobal(current_lua_state, "FilterLog");
    lua_pushstring(current_lua_state, source);
    lua_pushinteger(current_lua_state, linenumber);
    if (lua_pcall(current_lua_state, 2, 1, 0)) {
        lua_pop(current_lua_state, 1);
        return true;
    }

    auto ret = lua_toboolean(current_lua_state, -1);
    lua_pop(current_lua_state, 1);

    return ret;
}

int print_hook(lua_State* L)
{
    int print_args = lua_gettop(L);

    lua_Debug debug{};
    lua_getstack(L, 1, &debug);
    lua_getinfo(L, "Sl", &debug);

    if (np::do_log_filtering && !filter_log(debug.source, debug.currentline))
        return 0;

    std::string source_info =
        std::string{"["} + debug.source + ":" + std::to_string(debug.currentline) + "]";

    lua_pushstring(L, source_info.c_str());

    lua_insert(L, 1);
    lua_settop(L, print_args + 1);

    return original_print_func(L);
}

}

namespace np {

bool do_log_filtering = false;

void install_extended_logs_hook(lua_State* L)
{
    print_func = get_lua_c_binding(L, "print");
    if (!print_func)
        return;

    MH_CreateHook(
        (void*)print_func,
        (void*)print_hook,
        (void**)&original_print_func
    );
}

void enable_extended_logging_hook()
{
    if (!print_func)
        return;

    MH_EnableHook((void*)print_func);
}

void disable_extended_logging_hook()
{
    if (!print_func)
        return;

    MH_DisableHook((void*)print_func);
}

}
