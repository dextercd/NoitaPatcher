dofile_once("data/scripts/debug/keycodes.lua")
dofile_once("mods/NoitaPatcher/load.lua")

local world_ffi = require("noitapatcher.nsew.world_ffi")

local picture = [[
                        .8
                      .888
                    .8888'
                   .8888'
                   888'
                   8'
      .88888888888. .88888888888.
   .8888888888888888888888888888888.
 .8888888888888888888888888888888888.
.&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&'
&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&'
&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&'
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@:
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@:
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.
`%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.
 `00000000000000000000000000000000000'
  `000000000000000000000000000000000'
   `0000000000000000000000000000000'
     `###########################'
jgs    `#######################'
         `#########''########'
           `""""""'  `"""""'
]]

function OnWorldPreUpdate()
    if InputIsMouseButtonJustDown(Mouse_left) then
        local grid_world =  world_ffi.get_grid_world()
        local chunk_map = grid_world.vtable.get_chunk_map(grid_world)

        local snow_ptr = world_ffi.get_material_ptr(CellFactory_GetType("snow_static"))

        local sx, sy = DEBUG_GetMouseWorld()
        local x, y = sx, sy

        for i=1,#picture do
            local ch = picture:sub(i, i)

            if ch == "\n" then
                x = sx
                y = y + 1
                goto continue
            end

            if ch ~= " " then
                local pcell = world_ffi.get_cell(chunk_map, x, y)
                if pcell[0] == nil then
                    local pixel = world_ffi.construct_cell(grid_world, x, y, snow_ptr, nil)
                    pcell[0] = pixel
                end
            end

            x = x + 1

            ::continue::
        end
    end
end
