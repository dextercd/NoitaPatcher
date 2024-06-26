extern "C" {
#include <lua.h>
}
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
    float blood_multiplier;
    int projectile_thats_responsible;
    const char* description;
};

std::vector<DamageDetails> damage_details;

struct DamageThing {
    void* entity;
    char damage_by_type[0x40];
    vec2 impulse;
    vec2 world_pos;
    float knockback_force;
    int entity_thats_responsible;
    int entity_type_id;
    char unknown1[4];
    int projectile_thats_responsible;
    unsigned ragdoll_fx;
    char unknown2[4];
    vs13::string ragdoll_entity_file;
    vs13::string str_tags_q;
    char unknown3[8];
    float blood_multiplier;
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

    damage_details.push_back({
        .damage_types = damage_types,
        .ragdoll_fx = damage_args->ragdoll_fx,
        .impulse = damage_args->impulse,
        .world_pos = damage_args->world_pos,
        .knockback_force = damage_args->knockback_force,
        .blood_multiplier = damage_args->blood_multiplier,
        .projectile_thats_responsible = damage_args->projectile_thats_responsible,
        .description = description.c_str(),
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
    lua_createtable(L, 0, 8);

    lua_pushinteger(L, details.damage_types);
    lua_setfield(L, -2, "damage_types");

    lua_pushinteger(L, details.ragdoll_fx);
    lua_setfield(L, -2, "ragdoll_fx");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, details.impulse.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, details.impulse.y);
    lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "impulse");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, details.world_pos.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, details.world_pos.y);
    lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "world_pos");

    lua_pushinteger(L, details.knockback_force);
    lua_setfield(L, -2, "knockback_force");

    lua_pushnumber(L, details.blood_multiplier);
    lua_setfield(L, -2, "blood_multiplier");

    lua_pushinteger(L, details.projectile_thats_responsible);
    lua_setfield(L, -2, "projectile_thats_responsible");

    lua_pushstring(L, details.description);
    lua_setfield(L, -2, "description");

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
