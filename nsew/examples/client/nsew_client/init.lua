-- LuaSocket
package.path = package.path .. ";.\\mods\\nsew_client\\ldir\\?.lua"
package.cpath = package.cpath .. ";.\\mods\\nsew_client\\cdir\\?.dll"

local ffi = require("ffi")
local socket = require("socket")

-- NSEW
dofile_once("mods/nsew_client/deps/nsew/load.lua")("mods/nsew_client/deps")

local world_ffi = require("nsew.world_ffi")
local world = require("nsew.world")
local rect = require("nsew.rect")
local rect_optimiser = rect.Optimiser_new()

dofile("data/scripts/lib/coroutines.lua")

ModMagicNumbersFileAdd("mods/nsew_client/files/magic_numbers.xml")

local master = socket.tcp()
assert(master:connect('127.0.0.1', 44174))
local connection = master

function set_colour(grid_world, x, y, col)
    local chunk_map = grid_world.vtable.get_chunk_map(grid_world)
    local ppixel = world_ffi.get_cell(chunk_map, x, y)
    if ppixel[0] == nil then
        return
    end

    if ppixel[0].vtable ~= 0x00e21dc0 then
        return
    end

    -- the colour fields is argb using individual bytes. But the definition we
    -- use in this code here type-puns it to a little endian integer. Reorder
    -- the bytes so the colour is correct
    col = bit.rshift(bit.bswap(col), 8)
    ppixel[0].colour = bit.bor(bit.band(ppixel[0].colour, 0xff000000), col)
end

function get_player()
    return EntityGetWithTag("player_unit")[1]
end

function get_cursor_position()
    local x, y = DEBUG_GetMouseWorld()
    return x, y
end

function left_pressed()
    local control = EntityGetFirstComponent(get_player(), "ControlsComponent")
    return (ComponentGetValue2(control, "mButtonDownFire") and ComponentGetValue2(control, "mButtonDownFire2"))
end

function right_pressed()
    local control = EntityGetFirstComponent(get_player(), "ControlsComponent")
    return ComponentGetValue2(control, "mButtonDownThrow")
end


local encoded_area = world.EncodedArea()

function send_world_part(chunk_map, start_x, start_y, end_x, end_y)
    local area = world.encode_area(chunk_map, start_x, start_y, end_x, end_y, encoded_area)
    if area == nil then
        return
    end

    local str = ffi.string(area, world.encoded_size(area))
    send_str(str)
end

function send_str(str)
    connection:settimeout(nil)

    local index = 1
    while index ~= #str do
        local new_index, err, partial_index = connection:send(str, index)
        if new_index == nil then
            print("For str with total length " .. #str .. "We sent from index " .. index .. " new index " ..
                      partial_index)
            print("Error " .. err)
            index = partial_index
        else
            index = new_index
        end
    end
end

function do_receive()
    while receive_one() do
    end
end

local header_buffer = nil
local current_header = nil
local partial = ''

local PixelRun_const_ptr = ffi.typeof("struct PixelRun const*")

function receive_one()
    if current_header == nil then
        connection:settimeout(0)
        local received, err, part = connection:receive(ffi.sizeof(world.EncodedAreaHeader) - #partial)

        if received == nil then
            partial = partial .. part
            return false
        end

        received = partial .. received
        partial = ''

        header_buffer = received

        current_header = ffi.cast("struct EncodedAreaHeader const*", ffi.cast('char const*', header_buffer))
    end

    local body_size = ffi.sizeof(world.PixelRun) * current_header.pixel_run_count
    local data_left = body_size - #partial

    local received, err, part = connection:receive(tonumber(data_left))
    if received == nil then
        partial = partial .. part
        return false
    end
    received = partial .. received
    partial = ''

    local header = current_header
    current_header = nil
    header_buffer = nil

    local grid_world = world_ffi.get_grid_world()
    local runs = ffi.cast(PixelRun_const_ptr, ffi.cast("const char*", received))
    world.decode(grid_world, header, runs)

    return true
end

function OnWorldPreUpdate()
    wake_up_waiting_threads(1)

    if connection == nil then
        return
    end

    if os.getenv("ROLE") == "R" then
        do_receive()
        return
    end

    local grid_world = world_ffi.get_grid_world()
    local chunk_map = grid_world.vtable.get_chunk_map(grid_world)
    local thread_impl = grid_world.mThreadImpl

    local begin = thread_impl.updated_grid_worlds.begin
    local end_ = begin + thread_impl.chunk_update_count

    local count = thread_impl.chunk_update_count

    for i = 0, count - 1 do
        local it = begin[i]

        local start_x = it.update_region.top_left.x
        local start_y = it.update_region.top_left.y
        local end_x = it.update_region.bottom_right.x
        local end_y = it.update_region.bottom_right.y

        start_x = start_x - 1
        start_y = start_y - 1
        end_x = end_x + 1
        end_y = end_y + 2

        local rectangle = rect.Rectangle(start_x, start_y, end_x, end_y)
        rect_optimiser:submit(rectangle)
    end

    for i = 0, tonumber(thread_impl.world_update_params_count) - 1 do
        local wup = thread_impl.world_update_params.begin[i]
        local start_x = wup.update_region.top_left.x
        local start_y = wup.update_region.top_left.y
        local end_x = wup.update_region.bottom_right.x
        local end_y = wup.update_region.bottom_right.y

        local rectangle = rect.Rectangle(start_x, start_y, end_x, end_y)
        rect_optimiser:submit(rectangle)

    end

    if GameGetFrameNum() % 1 == 0 then
        rect_optimiser:scan()

        local result = ''
        for rect in rect.parts(rect_optimiser:iterate(), 256) do
            local area = world.encode_area(chunk_map, rect.left, rect.top, rect.right, rect.bottom, encoded_area)
            if area ~= nil then
                local str = ffi.string(area, world.encoded_size(area))
                result = result .. str
            end
        end
        send_str(result)

        rect_optimiser:reset()
    end
end

function OnPlayerSpawned(player_entity)
    if true then
        return
    end
    async(function()
        -- Only the changed world data gets sent, which isn't very interesting to look at,
        -- so just send a bunch of data around the player

        -- Ensure chunks are loaded around the player
        wait(60)

        local grid_world = world_ffi.get_grid_world()
        local chunk_map = grid_world.vtable.get_chunk_map(grid_world)

        for y = -2048, 2048, 64 do
            for x = -2048, 2048, 64 do
                send_world_part(chunk_map, x, y, x + 64, y + 64)
            end
        end
    end)
end
