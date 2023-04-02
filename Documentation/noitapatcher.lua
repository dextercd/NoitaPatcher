---@module noitapatcher
local noitapatcher = {}

---Sets Noita's internal RNG state to the specified value.
---This RNG state is used for many things including setting a fired projectile's
---direction based on random spread.
---@param rng_value integer New RNG state value
function noitapatcher.SetProjectileSpreadRNG(rng_value) end

---Disable the red flash upon taking damage for all entities except the one specified by entity_id.
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

return noitapatcher
