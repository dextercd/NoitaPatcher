#include "lua_util.hpp"
#include <iostream>

#include <utility>

lua_CFunction get_lua_c_binding(lua_State* L, const char* func_name)
{
    lua_getglobal(L, func_name);
    auto f = lua_tocfunction (L, -1);
    lua_pop(L, 1);
    return f;
}

std::optional<stack_entry> get_stack_entry(lua_State* L, int level)
{
    lua_Debug debug{};
    if (!lua_getstack(L, level, &debug))
        return std::nullopt;

    if (!lua_getinfo(L, "Snl", &debug))
        return std::nullopt;

    return stack_entry{
        .source = debug.source,
        .function_name = (debug.name ? debug.name : "<unknown func>"),
        .line_number = debug.currentline,
    };
}

std::vector<stack_entry> get_stack_trace(lua_State* L, int start_level, int count)
{
    std::vector<stack_entry> trace;

    for (int level = start_level; true; ++level) {
        if (count != -1 && count-- == 0)
            break;

        auto stack = get_stack_entry(L, level);
        if (!stack)
            break;

        trace.push_back(*stack);
    }

    return trace;
}

std::string_view ulua_checkstringview(lua_State* L, int narg)
{
    std::size_t len;
    auto str = lua_tolstring(L, narg, &len);

    if (!str) return "";
    return {str, len};
}
