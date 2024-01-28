#ifndef NP_CALLING_CONVENTION_HPP
#define NP_CALLING_CONVENTION_HPP

#ifdef __GNUC__

#define GET_FASTCALL_REGISTER_ARGS(a, b) asm("" : "=c"(a), "=d"(b))
#define STACK_ADJUST(n) asm("add $" #n ", %esp")
#define FLOAT_FROM_REGISTER(dst, reg) asm("movss %%" #reg ", %0" : "=m"(dst) :);
#define FLOAT_TO_REGISTER(reg, src) asm("movss %0, %%" #reg : : "m"(src));

#define GET_ECX(into) asm("": "=c"(into))

#else // MSVC

#define GET_FASTCALL_REGISTER_ARGS(a, b) \
    __asm {              \
        __asm mov a, ecx \
        __asm mov b, edx \
    }

#define STACK_ADJUST(n) __asm { add esp, n }

#define FLOAT_FROM_REGISTER(dst, reg) __asm { __asm movss dst, reg }
#define FLOAT_TO_REGISTER(reg, src) __asm { __asm movss reg, src }

#define GET_ECX(into) \
    __asm {              \
        __asm mov into, ecx \
    }

#endif

#endif // Header guard
