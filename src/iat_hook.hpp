#ifndef NP_IAT_HOOK
#define NP_IAT_HOOK

struct iat_hook {
    void** location = nullptr;
    void* original = nullptr;
    void* replacement = nullptr;
    bool enabled = false;

    void enable();
    void disable();

    void write(void*);
};

#endif // Header guard
