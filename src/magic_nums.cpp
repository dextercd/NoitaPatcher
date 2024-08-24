#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "executable_info.hpp"
#include "lua_util.hpp"
#include "memory_pattern.hpp"
#include <vs2013/string.hpp>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

enum class MagicNumType {
    bool_,
    int_,
    uint,
    float_,
    double_,
    string,
    unknown,
};

const char* to_cstr(MagicNumType type)
{
    switch (type) {
        case MagicNumType::bool_: return "bool";
        case MagicNumType::int_: return "int";
        case MagicNumType::uint: return "uint";
        case MagicNumType::float_: return "float";
        case MagicNumType::double_: return "double";
        case MagicNumType::string: return "string";
        case MagicNumType::unknown: return "unknown";
    }

    throw std::runtime_error{"Unhandled switch case"};
}

struct MagicNumEntry {
    MagicNumType type;
    void* address;
};

struct MagicNumbersDb {
    std::unordered_map<std::string, MagicNumEntry> magic_numbers;
    std::vector<std::string> num_order;
};

std::optional<MagicNumbersDb> make_magic_numbers_db(lua_State* L)
{
    MagicNumbersDb magic_nums_db;

    auto noita = ThisExecutableInfo::get();
    auto get_value_fun = (const char*)get_lua_c_binding(L, "MagicNumbersGetValue");

    if (!get_value_fun) {
        std::cerr << "Couldn't find 'MagicNumbersGetValue' Lua function.\n";
        return std::nullopt;
    }

    auto get_magic_num_pattern = make_pattern(
        Bytes{0x00, 0x00, 0x00, 0x8d, 0x4c, 0x24}, Pad{1},
        Bytes{0xe8}, Capture{"GetMagicNum", 4},
        Bytes{0x8d}
    );

    auto get_magic_num_result = get_magic_num_pattern.search(
        noita,
        get_value_fun,
        get_value_fun + 2000
    );

    if (!get_magic_num_result) {
        std::cerr << "Couldn't find CAnyContainer 'GetMagicNum' function.\n";
        return std::nullopt;
    }

    auto get_magic_num = get_magic_num_result.get_rela_call("GetMagicNum");
    auto search_start = (const char*)get_magic_num;

    auto magic_num_pattern = make_pattern(
        Bytes{0xba}, Capture{"Name", 4},
        Bytes{0x8b, 0xcf},
        Bytes{0xe8}, Pad{4}, // call std_string_cmp_c_str
        Bytes{0x84, 0xc0},   // test al, al
        Bytes{0x74}, Pad{1}, // jz
        Bytes{0x68}, Capture{"Address", 4},
        Bytes{0x8b, 0xce},
        Bytes{0xe8}, Capture{"ContainerFromDataType", 4}
    );

    struct CollectEntry {
        std::string_view name;
        void* address;
        const void* data_type_func;
    };
    std::vector<CollectEntry> found_nums;

    void* container_from_bool = nullptr;
    void* container_from_int = nullptr;
    void* container_from_uint = nullptr;
    void* container_from_float = nullptr;
    void* container_from_double = nullptr;
    void* container_from_string = nullptr;

    while (true) {
        auto magic_num_result = magic_num_pattern.search(
            noita,
            search_start,
            search_start + 300
        );
        if (!magic_num_result)
            break;

        auto cname = magic_num_result.get<const char*>("Name");
        auto name = std::string_view{cname};
        auto address = magic_num_result.get<void*>("Address");
        auto data_type_func = magic_num_result.get_rela_call("ContainerFromDataType");

        // setup for next search
        search_start = (const char*)magic_num_result.ptr + 1;

        found_nums.push_back(CollectEntry{
            .name = name,
            .address = address,
            .data_type_func = data_type_func,
        });

        if (name == "REPORT_DAMAGE_TYPE") {
            container_from_bool = data_type_func;
        } else if (name == "NUM_ORBS_TOTAL") {
            container_from_int = data_type_func;
        } else if (name == "WORLD_SEED") {
            container_from_uint = data_type_func;
        } else if (name == "DROP_LEVEL_1") {
            container_from_float = data_type_func;
        } else if (name == "STREAMING_FREQUENCY") {
            container_from_double = data_type_func;
        } else if (name == "REPORT_DAMAGE_FONT") {
            container_from_string = data_type_func;
        }
    }

    if (found_nums.empty()) {
        std::cerr << "No magic numbers found. 'GetMagicNum' is wrong?\n";
        std::cerr << "- " << get_magic_num << "\n";
        return std::nullopt;
    }

    for (auto entry : found_nums) {
        auto type = MagicNumType::unknown;

        if (entry.data_type_func == container_from_bool) {
            type = MagicNumType::bool_;
        } else if (entry.data_type_func == container_from_int) {
            type = MagicNumType::int_;
        } else if (entry.data_type_func == container_from_uint) {
            type = MagicNumType::uint;
        } else if (entry.data_type_func == container_from_float) {
            type = MagicNumType::float_;
        } else if (entry.data_type_func == container_from_double) {
            type = MagicNumType::double_;
        } else if (entry.data_type_func == container_from_string) {
            type = MagicNumType::string;
        }

        magic_nums_db.magic_numbers[std::string{entry.name}] = MagicNumEntry{
            .type = type,
            .address = entry.address,
        };

        magic_nums_db.num_order.push_back(std::string{entry.name});
    }

    return magic_nums_db;
}

MagicNumbersDb* get_magic_numbers_db(lua_State* L)
{
    static bool tried = false;
    static std::optional<MagicNumbersDb> db;
    if (!tried) {
        tried = true;
        db = make_magic_numbers_db(L);
    }

    if (!db)
        return nullptr;

    return &db.value();
}

int lua_MagicNumbersSetValue(lua_State* L)
{
    if (lua_gettop(L) != 2) {
        return luaL_error(L, "Expected exactly two arguments");
    }

    auto magic_nums_db = get_magic_numbers_db(L);
    if (!magic_nums_db)
        return luaL_error(L, "Couldn't find magic numbers in exe");

    auto name = luaL_checklstring(L, 1, nullptr);
    auto entry = magic_nums_db->magic_numbers.find(name);
    if (entry == std::end(magic_nums_db->magic_numbers))
        return luaL_error(L, "Couldn't find magic number '%s'", name);

    auto& magic_num = entry->second;

    if (magic_num.type == MagicNumType::bool_) {
        auto value = lua_toboolean(L, 2);
        *(bool*)magic_num.address = value;
    } else if (magic_num.type == MagicNumType::int_) {
        auto value = luaL_checkinteger(L, 2);
        *(int*)magic_num.address = value;
    } else if (magic_num.type == MagicNumType::uint) {
        auto value = luaL_checknumber(L, 2);
        *(unsigned int*)magic_num.address = value;
    } else if (magic_num.type == MagicNumType::float_) {
        auto value = luaL_checknumber(L, 2);
        *(float*)magic_num.address = value;
    } else if (magic_num.type == MagicNumType::double_) {
        auto value = luaL_checknumber(L, 2);
        *(double*)magic_num.address = value;
    } else if (magic_num.type == MagicNumType::string) {
        size_t length;
        auto value = luaL_checklstring(L, 2, &length);
        *(vs13::string*)magic_num.address = vs13::string{value, length};
    } else {
        return luaL_error(
            L,
            "Magic number '%s' has unsupported data type '%s' for assignment",
            name,
            to_cstr(magic_num.type)
        );
    }

    return 0;
}

int lua_MagicNumbersGetList(lua_State* L)
{
    auto magic_nums_db = get_magic_numbers_db(L);
    if (!magic_nums_db)
        return luaL_error(L, "Couldn't find magic numbers in exe");

    lua_createtable(L, magic_nums_db->magic_numbers.size(), 0);

    int index = 1;
    for (auto&& name : magic_nums_db->num_order) {
        auto& num = magic_nums_db->magic_numbers[name];
        lua_createtable(L, 0, 4);
        lua_pushstring(L, name.c_str());
        lua_setfield(L, -2, "name");
        lua_pushinteger(L, (std::intptr_t)num.address);
        lua_setfield(L, -2, "address");
        lua_pushstring(L, to_cstr(num.type));
        lua_setfield(L, -2, "type");

        lua_rawseti(L, -2, index++);
    }

    return 1;
}
