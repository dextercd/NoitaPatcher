#include <cstdint>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "components.hpp"
#include "noita.hpp"
#include "ecs.hpp"

namespace np {

int lua_GetComponentAddress(lua_State* L)
{
    auto comp = np::ComponentById(luaL_checkinteger(L, 1));
    lua_pushnumber(L, (std::uintptr_t)comp);
    return 1;
}

int lua_GetEntityAddress(lua_State* L)
{
    auto ent = entity_get_by_id(entity_manager, luaL_checkinteger(L, 1));
    lua_pushnumber(L, (std::uintptr_t)ent);
    return 1;
}

} // namespace np
