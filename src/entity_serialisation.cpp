#include <cstdint>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "noita.hpp"
#include "calling_convention.hpp"

#include "serializer.hpp"

namespace np {

using serialise_entity_function_t = void(__fastcall*)(const Entity*, SerialSaver*);
using deserialise_entity_function_t = void(__fastcall*)(Entity*, SerialLoader*, const vec2*);

const void* serialise_entity_func = nullptr;
const void* deserialise_entity_func = nullptr;

int SerializeEntity(lua_State* L)
{
    if (!serialise_entity_func)
        return 0;

    auto func = reinterpret_cast<serialise_entity_function_t>(serialise_entity_func);

    int entity_id = luaL_checkinteger(L, 1);
    auto entity = entity_get_by_id(entity_manager, entity_id);

    SerialSaver saver;

    func(entity, &saver);

    lua_pushlstring(L, saver.buffer.c_str(), saver.buffer.size());
    return 1;
}

int DeserializeEntity(lua_State* L)
{
    if (!deserialise_entity_func)
        return 0;

    auto func = reinterpret_cast<deserialise_entity_function_t>(deserialise_entity_func);

    int entity_id = luaL_checkinteger(L, 1);
    auto entity = entity_get_by_id(entity_manager, entity_id);

    if (!entity) {
        return 0;
    }

    std::size_t serialized_length;
    auto serialized_data = lua_tolstring(L, 2, &serialized_length);

    if (serialized_length < 40) {
        // Seems like a newly created entity using `EntityCreateNew()` is
        // already 48 bytes.. An incorrect serialisation string crashes the
        // game, so let's put a safeguard here.
        return 0;
    }

    vec2 position;
    vec2* has_position = nullptr;
    if (lua_gettop(L) >= 4) {
        position.x = lua_tonumber(L, 3);
        position.y = lua_tonumber(L, 4);
        has_position = &position;
    }

    SerialLoader loader{serialized_data, serialized_length};
    func(entity, &loader, has_position);
    STACK_ADJUST(4);

    // Return the entity to support:
    // local ent = np.DeserializeEntity(EntityCreateNew(), s)
    lua_pushinteger(L, entity_id);
    return 1;
}

}
