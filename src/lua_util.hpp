#ifndef NP_LUA_UTIL_HPP
#define NP_LUA_UTIL_HPP

extern "C" {
#include <lua.h>
}

#include <optional>
#include <string>
#include <vector>

struct stack_entry {
    std::string source;
    std::string function_name;
    int line_number;
};

lua_CFunction get_lua_c_binding(lua_State* L, const char* func_name);

std::optional<stack_entry> get_stack_entry(lua_State* L, int level);
std::vector<stack_entry> get_stack_trace(lua_State* L, int start_level, int count = -1);

#endif // Header guard
