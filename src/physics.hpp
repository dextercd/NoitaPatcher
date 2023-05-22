#ifndef NP_PHYSICS_HPP
#define NP_PHYSICS_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace np {

int PhysBodySetTransform(lua_State*);
int PhysBodyGetTransform(lua_State*);

}

#endif // Header guard
