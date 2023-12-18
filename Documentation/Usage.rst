Usage
-----

To use this API extension, your mod must have ``request_no_api_restrictions="1"``
specified in its ``mod.xml`` file. You must place the ``NoitaPatcher`` directory
somewhere in your mod's file tree.

Given a mod with the following file structure::

    Hamis/
    ├── mod.xml
    ├── init.lua
    └── NoitaPatcher
        ├── load.lua
        └── noitapatcher
            ├── noitapatcher.dll
            └── nsew
                └── ...

You would load NoitaPatcher with the following code:

.. code-block:: lua

    dofile_once("mods/Hamis/NoitaPatcher/load.lua")
    local np = require("noitapatcher")

You can now use ``np`` to access the :lua:mod:`noitapatcher` functionality.

See :ref:`example` for a complete example mod that uses NoitaPatcher.
