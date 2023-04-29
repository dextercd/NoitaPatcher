#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "iat_hook.hpp"

void iat_hook::enable()
{
    if (enabled)
        return;

    write(replacement);
    enabled = true;
}

void iat_hook::disable()
{
    if (!enabled)
        return;

    write(original);
    enabled = false;
}

void iat_hook::write(void* value)
{
    DWORD restore;
    VirtualProtect(location, sizeof(value), PAGE_READWRITE, &restore);
    *location = value;
    VirtualProtect(location, sizeof(value), restore, &restore);
}
