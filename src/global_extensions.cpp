#include "global_extensions.hpp"

void GlobalExtensions::grant_extensions(lua_State* L)
{
    for (auto&& extension : extensions) {
        lua_pushcfunction(L, extension.function);
        lua_setglobal(L, extension.name);
    }
}
