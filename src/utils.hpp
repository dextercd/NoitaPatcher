#ifndef NOITA_PATCHER_UTILS_HPP
#define NOITA_PATCHER_UTILS_HPP

template<class MemFn>
void* memfn_voidp(MemFn memfn)
{
    return *(void**)&memfn;
}

#endif // Header guard
