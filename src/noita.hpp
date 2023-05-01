#ifndef NP_NOITA_HPP
#define NP_NOITA_HPP

#include "executable_info.hpp"

#include <vs2013/vector.hpp>
#include <vs2013/string.hpp>

struct vec2 {
    float x;
    float y;
};

struct EntityManager;
struct Entity;

struct NormalEntity {
    int EntityId;
};

struct DevEntity {
    void* vtable;
    int EntityId;
};

struct DeathMatch {
    void * application_vtable;
    void * mouse_listener_vtable;
    void * keyboard_listener_vtable;
    char unk;
    void * joystick_listener_vtable;
    void * simple_ui_listener_vtable;
    void * event_listener_vtable;
    char dont_know1[60];
    vs13::vector<Entity*> player_entities;
    char dont_know2[100];
};

static_assert(sizeof(void*) == 4);
static_assert(sizeof(DeathMatch) == 200);

struct ComponentUpdator;

struct ComponentUpdator_vtable {
    void (__thiscall* destructor)(ComponentUpdator* self, bool do_delete);
    void (__thiscall* Unk1)(ComponentUpdator* self);
    void (__thiscall* Unk3)(ComponentUpdator* self);
    void (__thiscall* update_components)(ComponentUpdator* self, void*);
    int (__thiscall* get_update_priority)(ComponentUpdator* self);
    void (__thiscall* Unk4)(ComponentUpdator* self);
    const vs13::string& (__thiscall* get_system_name)(ComponentUpdator* self);
    void (__thiscall* Unk5)(ComponentUpdator* self);
    void (__thiscall* set_component_buffer)(ComponentUpdator* self, void*);
    void* (__thiscall* get_component_buffer)(ComponentUpdator* self);
    void (__thiscall* Unk6)(ComponentUpdator* self);
    void (__thiscall* Unk7)(ComponentUpdator* self);
    void (__thiscall* Unk8)(ComponentUpdator* self);
    void (__thiscall* Unk9)(ComponentUpdator* self);
    void (__thiscall* Unk10)(ComponentUpdator* self);
    void (__thiscall* Unk11)(ComponentUpdator* self);
    void (__thiscall* Unk12)(ComponentUpdator* self);
    bool (__thiscall* is_multi_threaded)(ComponentUpdator* self);
    void (__thiscall* Unk13)(ComponentUpdator* self);
    void (__thiscall* Unk14)(ComponentUpdator* self);
};

struct ComponentUpdator {
    ComponentUpdator_vtable* vtable;
};

struct SystemManager {
    int a;
    int b;
    vs13::vector<void*> mSystemAutoCreators;
    vs13::vector<ComponentUpdator*> mSystems;
};


// Note: It's really a mix between __fastcall (uses registers) and
// __cdecl (stack cleanup is done by caller).
using fire_wand_function_t =
    void(__fastcall*)(
        Entity* shooter,
        Entity* verlet_parent,
        const vec2& position,
        Entity* projectile,
        int unknown1,
        int unknown2,
        char unknown3,
        bool send_message,
        float target_x,
        float target_y
    );

// Note: It's realy __thiscall but we don't need to care about the system (ECS)
using platform_shooter_damage_message_handler_t =
    void(__stdcall*)(
        Entity* entity,
        void* unknown,
        void* unknown2
    );

using entity_get_by_id_t =
    Entity* (__thiscall*)(EntityManager*, int entity_id);

using set_active_held_entity_t =
    void (__fastcall*)(Entity* entity, Entity* item_entity, bool unk, bool make_noise);

using call_scripts_on_pause_pre_update_t =
    void (__stdcall*)();

using update_component_t =
    void (__thiscall*)(void* sys, Entity* entity, void* component);

inline int EntityGetId(Entity* entity)
{
    executable_info noita = ThisExecutableInfo::get();

    if (noita.is_dev_build)
        return ((DevEntity*)entity)->EntityId;

    return ((NormalEntity*)entity)->EntityId;
}

struct Message_UseItem {
    void* vtable;
    int unknown;
    int strings[3];
    bool mIgnoreReload;
    bool mCharge;
    bool mStartedUsingThisFrame;
    vec2 mPos;
    vec2 mTarget;
};

using send_message_use_item_t = void(__stdcall*)(Entity*, Message_UseItem*);

#endif // Header guard
