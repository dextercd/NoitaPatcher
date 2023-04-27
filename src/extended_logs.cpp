#include "lua_util.hpp"

extern "C" {
#include <lualib.h>
}

#include "extended_logs.hpp"
#include <MinHook.h>
#include <string>

namespace {

lua_CFunction print_func;
lua_CFunction original_print_func;

int print_hook(lua_State* L)
{
    int print_args = lua_gettop(L);

    lua_Debug debug{};
    lua_getstack(L, 1, &debug);
    lua_getinfo(L, "Sl", &debug);

    std::string source_info = debug.source;
    source_info += ":";
    source_info += std::to_string(debug.currentline);

    lua_pushstring(L, source_info.c_str());

    lua_insert(L, 1);
    lua_settop(L, print_args + 1);

    return original_print_func(L);
}

}

namespace np {

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
