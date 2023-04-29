#ifndef NP_EXTENDED_LOGS_HPP
#define NP_EXTENDED_LOGS_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "executable_info.hpp"

namespace np {

void enable_extended_logging_hook(const executable_info& exe, lua_State* L);
void disable_extended_logging_hook();

extern bool do_log_filtering;

}

#endif // Header guard
