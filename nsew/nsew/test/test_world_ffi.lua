local ffi = require("ffi")
local world_ffi = require("noitapatcher.nsew.world_ffi")

assert(ffi.sizeof("struct Cell") == 24)
assert(ffi.offsetof("struct Cell", "is_burning") == 16)
