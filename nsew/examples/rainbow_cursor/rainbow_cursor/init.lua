dofile_once("mods/rainbow_cursor/deps/nsew/load.lua")("mods/rainbow_cursor/deps")

local ffi = require 'ffi'
local world_ffi = require("nsew.world_ffi")

function get_cursor_position()
    local x, y = DEBUG_GetMouseWorld()
    return x, y
end

local colours = {
    world_ffi.Colour({0xff, 0x00, 0x00, 0xff}),
    world_ffi.Colour({0xff, 0xa5, 0x00, 0xff}),
    world_ffi.Colour({0xff, 0xff, 0x00, 0xff}),
    world_ffi.Colour({0x00, 0x08, 0x00, 0xff}),
    world_ffi.Colour({0x00, 0xff, 0xff, 0xff}),
    world_ffi.Colour({0x00, 0x99, 0xff, 0xff}),
    world_ffi.Colour({0x99, 0x00, 0xff, 0xff})
}

function OnWorldPostUpdate()
    local grid_world = world_ffi.get_grid_world()
    local chunk_map = grid_world.vtable.get_chunk_map(grid_world)

    local cx, cy = get_cursor_position()
    local offsets = {-2, -1, 0, 1, 2}
    for _, xoffset in ipairs(offsets) do
        for colnum, col in ipairs(colours) do
            local yoffset = #colours / 2 - colnum
            local x = ffi.cast('int32_t', cx + xoffset)
            local y = ffi.cast('int32_t', cy + yoffset)

            local pcell = world_ffi.get_cell(chunk_map, x, y)

            if pcell[0] ~= nil then
                local cell = pcell[0]
                cell.vtable.set_colour(cell, col)
            end
        end
    end
end
