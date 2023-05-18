#include <vector>
#include <iostream>

#include <vs2013/string.hpp>
#include <MinHook.h>

#include "noita.hpp"
#include "calling_convention.hpp"

#include "damage_detail.hpp"

namespace {

struct DamageDetails {
    unsigned damage_types;
    unsigned ragdoll_fx;
    vec2 impulse;
    vec2 world_pos;
    float knockback_force;
};

std::vector<DamageDetails> damage_details;

struct DamageThing {
    void* entity;
    char damage_by_type[0x3c];
    vec2 impulse;
    vec2 world_pos;
    float knockback_force;
    int entity_thats_responsible;
    int entity_type_id;
    char unknown1[8];
    unsigned ragdoll_fx;
    // bunch more stuff here
};

using inflict_damage_func_t = void (__fastcall*)(
    void* entity,
    void* damage_model,
    const vs13::string& description,
    unsigned damage_types,
    DamageThing* damage_args
);

inflict_damage_func_t original_inflict_damage;

void __cdecl inflict_damage_hook(
        const vs13::string& description,
        unsigned damage_types,
        DamageThing* damage_args)
{
    void* entity;
    void* damage_model;
    GET_FASTCALL_REGISTER_ARGS(entity, damage_model);

    float damage;
    FLOAT_FROM_REGISTER(damage, xmm2);

    damage_details.push_back(
        DamageDetails{
            damage_types,
            damage_args->ragdoll_fx,
            damage_args->impulse,
            damage_args->world_pos,
            damage_args->knockback_force
        });

    FLOAT_TO_REGISTER(xmm2, damage);
    original_inflict_damage(entity, damage_model, description, damage_types, damage_args);
    STACK_ADJUST(0xc);

    damage_details.pop_back();
}

}

namespace np {

int GetDamageDetails(lua_State* L)
{
    if (damage_details.empty())
        return 0;

    auto& details = damage_details.back();
    lua_createtable(L, 0, 5);

    lua_pushinteger(L, details.damage_types);
    lua_setfield(L, -2, "damage_types");

    lua_pushinteger(L, details.ragdoll_fx);
    lua_setfield(L, -2, "ragdoll_fx");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, details.impulse.x);
    lua_pushinteger(L, 1);
    lua_settable(L, -3);
    lua_pushnumber(L, details.impulse.y);
    lua_pushinteger(L, 2);
    lua_settable(L, -3);
    lua_setfield(L, -2, "impulse");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, details.world_pos.x);
    lua_pushinteger(L, 1);
    lua_settable(L, -3);
    lua_pushnumber(L, details.world_pos.y);
    lua_pushinteger(L, 2);
    lua_settable(L, -3);
    lua_setfield(L, -2, "world_pos");

    lua_pushinteger(L, details.knockback_force);
    lua_setfield(L, -2, "knockback_force");

    return 1;
}

void install_damage_detail_hook(void* inflict_damage_function)
{
    MH_CreateHook(
        inflict_damage_function,
        (void*)inflict_damage_hook,
        (void**)&original_inflict_damage
    );

    MH_EnableHook(inflict_damage_function);
}

}
