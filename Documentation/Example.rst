.. _example:

Example
-------

This is a mod that replaces the normal player entity with a Hämis by using the
:lua:func:`noitapatcher.SetPlayerEntity` function.

It's kind of jank but also fun. :^)

This should be the layout of the mod::

    Hamis/
    ├── mod.xml
    ├── init.lua
    └── NoitaPatcher
        ├── load.lua
        └── noitapatcher
            ├── noitapatcher.dll
            └── nsew
                └── ...

mod.xml
=======

.. literalinclude:: ../Examples/Hamis/mod.xml
   :language: xml

init.lua
========

.. literalinclude:: ../Examples/Hamis/init.lua
   :language: lua
   :linenos:

NoitaPatcher
================

https://github.com/dextercd/NoitaPatcher/releases
