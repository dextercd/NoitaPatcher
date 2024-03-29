callbacks
---------

NoitaPatcher adds some callbacks to whatever Lua context imports its module.

To use `OnProjectileFired` and `OnProjectileFiredPost` you must call
:lua:func:`noitapatcher.InstallShootProjectileFiredCallbacks`.

.. lua:function:: OnProjectileFired(shooter_id, projectile_id, rng, position_x, position_y, target_x, target_y, send_message, unknown1, multicast_index, unknown3)

   Called when a projectile is fired.

   The RNG that would be used is passed in, but at this point it can be modified
   by :lua:func:`noitapatcher.SetProjectileSpreadRNG`.

   This callback is mostly useful if you want to change the RNG, or if you want
   to synchronise the RNG value to other clients.

   :param shooter_id: Entity id of the entity that fired the projectile.
   :type shooter_id: integer

   :param projectile_id: Entity id of the projectile that was fired.
   :type projectile_id: integer

   :param rng: The current RNG state that will affect the velocity, direction, and other aspects of the projectile.
   :type rng: integer

   :param position_x: X position from which the projectile will be fired.
   :type position_x: number

   :param position_y: Y position from which the projectile will be fired.
   :type position_y: number

   :param target_x: X position towards which the projectile is fired.
   :type target_x: number

   :param target_y: Y position towards which the projectile is fired.
   :type target_y: number

   :param send_message: Will a message get sent to the DamageModel, GameEffect, DebugLogMessages, and Lua systems?

    This causes things like the 'shot' Lua callback to be called.
   :type send_message: boolean

   :param unknown1: 4 byte integer. Unknown what this is for. Maybe recursion_level
   :type unknown1: integer

   :param multicast_index: What number projectile is this in a multi-cast. -1 if multi-casts doesn't make sense.
   :type multicast_index: integer

   :param unknown3: 1 byte integer, probably actually a boolean. It's unknown what this is for.
   :type unknown3: integer


.. lua:function:: OnProjectileFiredPost(shooter_id, projectile_id, rng, position_x, position_y, target_x, target_y, send_message, unknown1, unknown2, unknown3)

   Exactly the same callback as :lua:func:`OnProjectileFired` except that it's
   called after Noita's internal GameShootProjectile function has run.
   At this point changing the RNG by calling :lua:func:`noitapatcher.SetProjectileSpreadRNG`
   won't have any effect on the projectile.

   This callback is mostly useful for when you want to extract the exact projectile
   parameters that were generated from the RNG. (e.g. velocity/direction.)


.. lua:function:: FilterLog(source, function_name, linenumber, ...)

   Decide whether the log should be performed or not. Only called once enabled
   by calling :lua:func:`noitapatcher.EnableLogFiltering`.

   :param source: Where does the log entry come from? Special value "=[C]" means
      there's no known source file.
   :type source: string

   :param function_name: The function that called print.
   :type function_name: string

   :param linenumber: Line number of the log call.
   :type linenumber: integer

   :param ...: All the arguments that are logged, passed in as separate arguments
   :type ...: string

   :return: Whether to perform the logging (true) or not (false).
   :rtype: boolean
