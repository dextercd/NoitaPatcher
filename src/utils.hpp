#ifndef NP_UTILS_HPP
#define NP_UTILS_HPP

template<class MemFn>
void* memfn_voidp(MemFn memfn)
{
    return *(void**)&memfn;
}

#endif // Header guard
