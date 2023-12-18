Package layout
==============

Examples/
    server/
    nsew_client
    rainbow_cursor/

These are example projects that use NoitaPatcher functionality.

    server: A simple TCP server supporting one way sync between two clients.

    nsew_client: A Noita mod that communicates with the server program. By
    default the mod writes world data. To receive world data you have to launch
    with the environment variable `ROLE=R` set.

    rainbow_cursor: Mod that changes the world under your cursor to rainbow
    colors.

To run the example mods you have to put the NoitaPatcher/ folder into your
Noita mods/ folder. For your own mod you would usually put the NoitaPatcher/
folder inside your mod.


Modules/
    noitapatcher.lua
    README.txt

This contains a .lua file that can be used for code completion. See the
README.txt in there for more details.


Documentation/

Contains documentation. Open the index.html file in a web browser to browse
them or go to https://dexter.döpping.eu/NoitaPatcher/ for the latest version of
the docs.


NoitaPatcher/

This is the actual library that you'll want to put inside your mod.
