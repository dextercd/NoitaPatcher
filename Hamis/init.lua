package.cpath = package.cpath .. ";./mods/Hamis/lib/?.dll"
local np = require("noitapatcher")

-- Must define these callbacks else you get errors every
-- time a projectile is fired. Functions are empty since
-- we don't use these callbacks at the moment.
function OnProjectileFired() end
function OnProjectileFiredPost() end

function OnPlayerSpawned(player_id)
    local x, y = EntityGetTransform(player_id)
    local hamis = EntityLoad("data/entities/animals/longleg.xml", x, y)

    -- Need to add a bunch of components for the entity
    -- to function more like the player.
    local player_comps = {
        "AudioListenerComponent",
        "GunComponent",
        "Inventory2Component",
        "InventoryGuiComponent",
        "ItemPickUpperComponent",
        "PlatformShooterPlayerComponent",
        "WalletComponent",
    }

    for _, comp in ipairs(player_comps) do
        EntityAddComponent2(hamis, comp)
    end

    local controls = EntityGetFirstComponent(hamis, "ControlsComponent")
    local damage = EntityGetFirstComponent(hamis, "DamageModelComponent")
    local inventory = EntityGetFirstComponent(hamis, "Inventory2Component")

    -- Setup inventory
    local inv_quick = EntityCreateNew("inventory_quick")
    local inv_full = EntityCreateNew("inventory_full")
    EntityAddChild(hamis, inv_quick)
    EntityAddChild(hamis, inv_full)

    ComponentSetValue2(inventory, "full_inventory_slots_x", 16)
    ComponentSetValue2(inventory, "full_inventory_slots_y", 1)

    -- Controllable by player
    ComponentSetValue2(controls, "enabled", true)

    -- Extra HP to stop low HP flashing
    ComponentSetValue2(damage, "hp", 4)
    ComponentSetValue2(damage, "max_hp", 4)

    -- Player tag
    EntityAddTag(hamis, "player_unit")

    -- Use NoitaPatcher to change what entity is considered
    -- to be the player.
    np.SetPlayerEntity(hamis)
    EntityKill(player_id)
end
