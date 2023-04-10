#ifndef NOITA_PATCHER_MEMORY_PATTERN_HPP
#define NOITA_PATCHER_MEMORY_PATTERN_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <memory>
#include <initializer_list>

struct executable_info;

struct Pad {
    int amount;
};

template<class T>
struct Raw {
    T value;
};

struct Capture {
    std::string name;
    int size;
};

struct Bytes {
    std::vector<char> data;

    template<std::size_t N>
    Bytes(const char (&bytes)[N])
        : data(std::begin(bytes), std::end(bytes))
    {
    }

    Bytes(std::vector<char> bytes)
        : data{std::move(bytes)}
    {
    }

    Bytes(std::initializer_list<std::uint8_t> init)
        : data(init.size())
    {
        std::memcpy(std::data(data), std::data(init), init.size());
    }
};

struct Call {
    std::uint32_t target;
};

struct CapturePosition {
    int offset;
    std::string name;
};

struct FindResult {
    const void* ptr = nullptr;
    std::vector<CapturePosition> captures;

    FindResult(std::nullptr_t) {}
    FindResult(const void* ptr_, std::vector<CapturePosition> captures_)
        : ptr{ptr_}
        , captures{std::move(captures_)}
    {
    }

    explicit operator bool() { return ptr; }

    void* capture_ptr(std::string_view capture_name);

    template<class T>
    T get(std::string_view capture_name)
    {
        T ret;
        void* location = capture_ptr(capture_name);
        std::memcpy(&ret, location, sizeof(ret));
        return ret;
    }

    void* get_rela_call(std::string_view capture_name)
    {
        auto location = (char*)capture_ptr(capture_name);
        auto offset = get<std::ptrdiff_t>(capture_name);
        // Relative calls are resolved relative to the *next* instruction
        auto next_instruction = location + sizeof(offset);
        return next_instruction + offset;
    }
};

struct PatternCheckArgs {
    const void* location;
    std::uint32_t load_location;
};

struct PatternCheck {
    virtual const void* search(const void* begin, const void* end) = 0;
    virtual bool check(const PatternCheckArgs& at) = 0;
    virtual ~PatternCheck() {}
};

struct Pattern {
    struct PatternPart {
        std::size_t offset;
        std::size_t size;
        std::shared_ptr<PatternCheck> check;
        // std::vector<char> bytes;
    };
    std::vector<PatternPart> parts;

    std::vector<CapturePosition> captures;
    int cursor = 0;

    ~Pattern();

    template<class T>
    void add(const Raw<T>& raw)
    {
        std::vector<char> bytes(sizeof(T));
        std::memcpy(&bytes[0], &raw.value, sizeof(T));
        add(Bytes{std::move(bytes)});
    }

    void add(Pad padding)
    {
        cursor += padding.amount;
    }

    void add(Capture capture)
    {
        captures.emplace_back(cursor, std::move(capture.name));
        add(Pad{capture.size});
    }

    void add(Bytes b);
    void add(Call call);

    std::size_t size()
    {
        return parts.back().offset + parts.back().size;
    }

    std::size_t following_size()
    {
        if (parts.size() < 2)
            return 0;

        return size() - parts[1].offset;
    }

    FindResult search(const executable_info& exe, const void* begin, const void* end);

private:
    FindResult make_result(const void* ptr);
};

template<class... Args>
Pattern make_pattern(Args&&... args)
{
    Pattern pat;
    auto dummy = {(pat.add(std::forward<Args>(args)), 0)...};
    return pat;
}

#endif // Header guard
