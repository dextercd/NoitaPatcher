#ifndef NP_VS13_MEMORY_HPP
#define NP_VS13_MEMORY_HPP

#include <cstddef>

namespace vs13 {

using operator_new_signature = void*(*)(std::size_t);
using operator_delete_signature = void(*)(void*);

extern operator_new_signature operator_new;
extern operator_delete_signature operator_delete;

}

#endif // Header guard
