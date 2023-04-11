#ifndef NP_GAME_PAUSE_HPP
#define NP_GAME_PAUSE_HPP

#include "x86.hpp"

struct executable_info;

extern bool disable_inventory_gui;
extern bool disable_player_item_pick_upper;

struct GamePauseData {
    function_range do_pause_update_callbacks;
    const void* call_pause_addr;
    function_range deathmatch_update;
    function_range update_inventory_gui;
    function_range update_item_pick_upper;
};

GamePauseData get_game_pause_data(const executable_info& info);
void disable_game_pause(const executable_info& exe, const GamePauseData& game_pause);

#endif // Header guard
