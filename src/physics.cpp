#include "components.hpp"

#include "physics.hpp"

namespace np {

int PhysBodySetTransform(lua_State* L)
{
    int component_id = luaL_checkinteger(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double r = luaL_checknumber(L, 4);
    double vx = luaL_checknumber(L, 5);
    double vy = luaL_checknumber(L, 6);
    double av = luaL_checknumber(L, 7);

    auto component = ComponentById(component_id);
    if (!component) {
        luaL_error(L, "Component not found.");
    }

    auto b2body = *(char**)((char*)component + 0x48);
    
    *(double*)(b2body + 0x58) = x;
    *(double*)(b2body + 0x60) = y;
    *(double*)(b2body + 0x70) = r;
    *(double*)(b2body + 0x80) = vx;
    *(double*)(b2body + 0x88) = vy;
    *(double*)(b2body + 0x90) = av;

    return 0;
}

int PhysBodyGetTransform(lua_State* L)
{
    int component_id = luaL_checkinteger(L, 1);

    auto component = ComponentById(component_id);
    if (!component) {
        luaL_error(L, "Component not found.");
    }

    auto b2body = *(char**)((char*)component + 0x48);
    
    lua_pushnumber(L, *(double*)(b2body + 0x58)); // x
    lua_pushnumber(L, *(double*)(b2body + 0x60)); // y
    lua_pushnumber(L, *(double*)(b2body + 0x70)); // r
    lua_pushnumber(L, *(double*)(b2body + 0x80)); // vx
    lua_pushnumber(L, *(double*)(b2body + 0x88)); // vy
    lua_pushnumber(L, *(double*)(b2body + 0x90)); // av

    return 6;
}

}
