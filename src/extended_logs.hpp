#ifndef NP_EXTENDED_LOGS_HPP
#define NP_EXTENDED_LOGS_HPP

#include "executable_info.hpp"

namespace np {

void install_extended_logs_hook(const executable_info& exe, lua_State* state);
void enable_extended_logging_hook();
void disable_extended_logging_hook();

extern bool do_log_filtering;

}

#endif // Header guard
