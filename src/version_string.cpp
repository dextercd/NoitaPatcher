extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include "x86.hpp"
#include "executable_info.hpp"


bool version_string_did_search = false;
const char* version_string;
int lua_GetVersionString(lua_State* L)
{
    if (!version_string_did_search) {
        version_string_did_search = true;
        const auto& noita = ThisExecutableInfo::get();
        version_string = find_rdata_string(noita, "Noita - Build ");
    }

    if (!version_string) {
        return luaL_error(L, "Couldn't find version string.");
    }

    lua_pushstring(L, version_string);
    return 1;
}
