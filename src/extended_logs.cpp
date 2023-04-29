#include <MinHook.h>
#include <string>
#include <iostream>
#include <vector>

#include "lua_util.hpp"
#include "iat_hook.hpp"
#include "extended_logs.hpp"

extern lua_State* current_lua_state;

namespace {

lua_CFunction print_func;
lua_CFunction original_print_func;

iat_hook pcall_hook;

bool filter_log(const stack_entry& stack, const std::vector<std::string>& printed_strings)
{
    lua_getglobal(current_lua_state, "FilterLog");
    lua_pushstring(current_lua_state, stack.source.c_str());
    lua_pushstring(current_lua_state, stack.function_name.c_str());
    lua_pushinteger(current_lua_state, stack.line_number);
    for (auto& str : printed_strings) {
        lua_pushstring(current_lua_state, str.c_str());
    }

    if (lua_pcall(current_lua_state, 3 + std::size(printed_strings), 1, 0)) {
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

    auto stack = get_stack_entry(L, 1).value();

    std::vector<std::string> printed_strings;
    for (int pi = 1; pi <= print_args; ++pi) {
        lua_pushvalue(L, pi);
        printed_strings.emplace_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    if (np::do_log_filtering && !filter_log(stack, std::move(printed_strings)))
        return 0;

    std::string source_info
        = "[" + stack.source
        + ":" + std::to_string(stack.line_number)
        + "]";

    lua_pushstring(L, source_info.c_str());

    lua_insert(L, -print_args);
    lua_settop(L, print_args + 1);

    return original_print_func(L);
}

int enhanced_pcall_error_handler(lua_State* L)
{
    auto stack_trace = get_stack_trace(L, 1);

    std::string textual_stack = lua_tostring(L, 1);
    for (auto& stack : stack_trace) {
        textual_stack +=
            "\n  " +
            stack.source + ':' +
            stack.function_name + ':' +
            std::to_string(stack.line_number);
    }

    lua_pushstring(L, textual_stack.c_str());

    return 1;
}

int enhanced_pcall(lua_State* L, int nargs, int nresults, int errfunc)
{
    if (errfunc)
        return lua_pcall(L, nargs, nresults, errfunc);

    errfunc = lua_gettop(L) - nargs;
    lua_pushcfunction(L, enhanced_pcall_error_handler);
    lua_insert(L, errfunc);

    auto ret = lua_pcall(L, nargs, nresults, errfunc);
    lua_remove(L, errfunc);

    return ret;
}

}

namespace np {

bool do_log_filtering = false;

void install_extended_logs_hook(const executable_info& exe, lua_State* L)
{
    auto pcall_iat = iat_address(exe, "lua51.dll", "lua_pcall");
    pcall_hook = {
        .location = pcall_iat,
        .original = *pcall_iat,
        .replacement = (void*)enhanced_pcall,
    };

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

    pcall_hook.enable();
    MH_EnableHook((void*)print_func);
}

void disable_extended_logging_hook()
{
    if (!print_func)
        return;

    MH_DisableHook((void*)print_func);
    pcall_hook.disable();
}

}
