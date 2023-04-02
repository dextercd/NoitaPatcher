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
    └── lib
        └── noitapatcher.dll

mod.xml
=======

.. literalinclude:: ../Hamis/mod.xml
   :language: xml

init.lua
========

.. literalinclude:: ../Hamis/init.lua
   :language: lua
   :linenos:

noitapatcher.dll
================

https://github.com/dextercd/NoitaPatcher/releases
