#ifndef NP_COMPONENTS_HPP
#define NP_COMPONENTS_HPP

namespace np {

struct ComponentMapNode {
    // std::map implementation detail
    void* left;
    void* parent;
    void* right;
    char color;
    char isnil;

    int component_id;
    void* component_ptr;
};

struct CompMapIt {
    ComponentMapNode* ptr;
    CompMapIt() {}
    bool operator==(const CompMapIt& other) { return ptr == other.ptr; }
    bool operator!=(const CompMapIt& other) { return !(*this == other); }
};

using get_component_by_id_t =
    CompMapIt*(__stdcall*)(CompMapIt* it, const int& component_id);

extern get_component_by_id_t get_component_by_id;

inline void* ComponentById(int id)
{
    CompMapIt it;
    get_component_by_id(&it, id);

    CompMapIt dummy;
    get_component_by_id(&dummy, -1);

    if (it == dummy)
        return nullptr;

    return it.ptr->component_ptr;
}

}

#endif // Header guard
