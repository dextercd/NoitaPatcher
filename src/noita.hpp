#ifndef NP_NOITA_HPP
#define NP_NOITA_HPP

#include "executable_info.hpp"

template<class T>
struct vs2013_vector {
    T* begin_;
    T* end_;
    T* capacity_end_;

    T* data() { return begin_; }

    T& front() { return *begin_; }
    T& back() { return *(end_ - 1); }

    std::size_t size() const { return end_ - begin_; }
    bool empty() const { return size() == 0; }

    T* begin() { return begin_; }
    T* end() { return end_; }
};

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
    vs2013_vector<Entity*> player_entities;
    char dont_know2[100];
};

static_assert(sizeof(void*) == 4);
static_assert(sizeof(DeathMatch) == 200);

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
