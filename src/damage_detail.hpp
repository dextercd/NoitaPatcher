#ifndef NP_DAMAGE_DETAIL
#define NP_DAMAGE_DETAIL

extern "C" {
#include <lua.h>
}

namespace np {

int GetDamageDetails(lua_State* L);
void install_damage_detail_hook(void* do_damage_function);

}

#endif // Header guard
