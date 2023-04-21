---@meta 'noitapatcher'
---@module noitapatcher

local noitapatcher = {}

---Sets Noita's internal RNG state to the specified value.
---This RNG state is used for many things including setting a fired projectile's
---direction based on random spread.
---@param rng_value integer New RNG state value
function noitapatcher.SetProjectileSpreadRNG(rng_value) end

---Disable the red flash upon taking damage for all entities with a PlatformShooterPlayerComponent except for the one specified by entity_id.
---You can restore the original behaviour by passing in -1 for the entity_id.
---@param entity_id integer ID of the only entity for which to do the damage flash.
function noitapatcher.RegisterPlayerEntityId(entity_id) end

---Change the item that the entity is holding.
---@param entity_id integer id of the entity for which you want to change what they are holding.
---@param item_id integer id of the entity that should be held. For the best effect it should be an item in the inventory_quick child of the entity specified by entity_id.
---@param unknown boolean Not sure what this does. Let me know if you find out!
---@param make_noise boolean Whether or not switching to this item should make a noise.
function noitapatcher.SetActiveHeldEntity(entity_id, item_id, unknown, make_noise) end

---Changes the entity that the game considers to be the player.
---This determines what entity is followed by the camera and whose death ends the game.
---A bunch more stuff is probably tied to this.
---@param entity_id integer The entity to make the game think of as the player.
function noitapatcher.SetPlayerEntity(entity_id) end

---Enables or disables game simulate pausing when opening escape or wand menu.
---You can only disable pausing at the moment, reenabling is not supported.
---@param enabled boolean Whether to enable or disable pausing.
function noitapatcher.EnableGameSimulatePausing(enabled) end

---Disable InventoryGuiComponent updates without disabling the component.
---Disabling updates for this component makes clicking on an empty wand slot work
---after using EnableGameSimulatePausing(false) and entering the wand pickup menu.
---@param enabled boolean Whether to enable or disable Inventory GUI updates.
function noitapatcher.EnableInventoryGuiUpdate(enabled) end

---Enable/disable ItemPickUpperComponent updates for the entity registerd using RegisterPlayerEntityId
---Disabling updates for this component prevents double wand cards from appearing
---after using EnableGameSimulatePausing(false) and entering the wand pickup menu.
---@param enabled boolean Whether to enable or disable ItemPickUpper updates.
function noitapatcher.EnablePlayerItemPickUpper(enabled) end

---Send a 'use item' message causing the item to get activated by the entity's ability component.
---@param responsible_entity_id integer Entity that should be seen as responsible for the item's use.
---@param item_entity_id integer Wand or other item entity.
---@param ignore_reload boolean _
---@param charge boolean _
---@param started_using_this_frame boolean _
---@param pos_x number _
---@param pos_y number _
---@param target_x number _
---@param target_y number _
function noitapatcher.UseItem(responsible_entity_id, item_entity_id, ignore_reload, charge, started_using_this_frame, pos_x, pos_y, target_x, target_y) end

---Patch out logging for a certain string literal.
---@param logstr string The string to look for in the exe, it should end with a newline character in most cases.
---@return bool patch_successful
function noitapatcher.SilenceLogs(logstr) end

return noitapatcher
