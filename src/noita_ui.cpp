#include <iostream>

#include <MinHook.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "memory_pattern.hpp"
#include "x86.hpp"
#include "calling_convention.hpp"

namespace {

using ui_grid_container_fn = void(__fastcall*)(int param_1, void* param_2_00,int param_3,int param_4,
          int param_5,int param_6,int param_7, void* param_8,
          int param_9, void* param_10,char param_11, unsigned param_12, void* param_13,
          int param_14, void* param_15,int param_16,float param_17, void* param_18);

ui_grid_container_fn original_grid_container = nullptr;

void __cdecl disable_drag_drop_hook(int param_3,int param_4,
          int param_5,int param_6,int param_7, void* param_8,
          int param_9, void* param_10,char param_11, unsigned param_12, void* param_13,
          int param_14, void* param_15,int param_16,float param_17, void* param_18)
{
    int param_1;
    void* param_2_00;
    GET_FASTCALL_REGISTER_ARGS(param_1, param_2_00);

    param_12 = 0x00200000;

    if (param_16 == 3)
        param_16 = 4;

    original_grid_container(param_1, param_2_00,param_3,param_4,
          param_5,param_6,param_7, param_8,
          param_9, param_10, param_11, param_12, param_13,
          param_14, param_15,param_16, param_17, param_18);

    STACK_ADJUST(0x40);
}


struct ui_grid_hook {
    bool succeeded = false;
    ui_grid_container_fn ui_grid_container;

    ui_grid_container_fn find_ui_grid_container()
    {
        const auto& noita = ThisExecutableInfo::get();
        auto pattern = make_pattern(Bytes{0x8b, 0x44, 0x24, 0x5c, 0x80, 0x78, 0x6d, 0x00, 0x8b, 0x44, 0x24, 0x34});
        auto result = pattern.search(noita, noita.text_start, noita.text_end);
        if (!result)
            return nullptr;

        return (ui_grid_container_fn)find_function_bounds(noita, result.ptr).start;
    }


    ui_grid_hook()
    {
        ui_grid_container = find_ui_grid_container();
        if (!ui_grid_container) {
            std::cerr << "Couldn't find UI grid container function.\n";
            return;
        }

        auto ch = MH_CreateHook(
            (void*)ui_grid_container,
            (void*)disable_drag_drop_hook,
            (void**)&original_grid_container
        );

        succeeded = ch == MH_OK;
    }

    void set_enabled(bool enable)
    {
        if (!succeeded)
            return;

        if (enable)
            MH_EnableHook((void*)ui_grid_container);
        else
            MH_DisableHook((void*)ui_grid_container);
    }
};

ui_grid_hook* get_grid_hook()
{
    static ui_grid_hook hook;
    if (hook.succeeded)
        return &hook;
    return nullptr;
}

} // namespace

namespace np {

int SetInventoryCursorEnabled(lua_State* L)
{
    bool enable = lua_toboolean(L, 1);
    auto grid_hook = get_grid_hook();
    if (!grid_hook) {
        luaL_error(L, "Couldn't hook ui grid container function");
    }

    // Enabling hook means disabling cursor
    grid_hook->set_enabled(!enable);
    return 0;
}

} // namespace np
