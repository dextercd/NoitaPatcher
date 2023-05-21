#ifndef NP_ENTITY_SERIALISATION_HPP
#define NP_ENTITY_SERIALISATION_HPP

extern "C" {
#include <lua.h>
}

namespace np {

extern const void* serialise_entity_func;
extern const void* deserialise_entity_func;

int SerializeEntity(lua_State* L);
int DeserializeEntity(lua_State* L);

}

#endif // Header guard
