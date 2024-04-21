extern "C" {
#include <lua.h>
}

namespace np {

void crosscall_reset();

int CrossCallAdd(lua_State* L);
int CrossCall(lua_State* L);

} // namespace np
