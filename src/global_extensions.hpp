#ifndef NP_GLOBAL_EXTENSIONS_HPP
#define NP_GLOBAL_EXTENSIONS_HPP

#include <vector>

extern "C" {
#include <lua.h>
}

struct Extension {
    const char* name;
    lua_CFunction function;
};

class GlobalExtensions {
    std::vector<Extension> extensions;

private:
    GlobalExtensions() = default;

public:
    static GlobalExtensions& instance()
    {
        static GlobalExtensions inst;
        return inst;
    }

    void add_extension(const char* name, lua_CFunction function)
    {
        extensions.emplace_back(name, function);
    }

    void grant_extensions(lua_State* L);
};

#endif
