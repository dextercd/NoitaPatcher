#ifndef NOITA_PATCHER_MEMORY_PATTERN_HPP
#define NOITA_PATCHER_MEMORY_PATTERN_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <initializer_list>

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

    Bytes(std::initializer_list<std::uint8_t> bytes)
        : data(bytes.size())
    {
        std::memcpy(std::data(data), std::data(bytes), bytes.size());
    }
};

struct CapturePosition {
    int offset;
    std::string name;
};

struct FindResult {
    void* ptr = nullptr;
    std::vector<CapturePosition> captures;

    FindResult(std::nullptr_t) {}
    FindResult(void* ptr_, std::vector<CapturePosition> captures_)
        : ptr{ptr_}
        , captures{captures_}
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

struct Pattern {
    struct PatternPart {
        std::size_t offset;
        std::vector<char> bytes;
    };
    std::vector<PatternPart> parts;

    std::vector<CapturePosition> captures;
    int cursor = 0;

    template<class T>
    void add(const Raw<T>& raw)
    {
        std::vector<char> bytes(sizeof(T));
        std::memcpy(&bytes[0], &raw.value, sizeof(T));
        parts.emplace_back(cursor, std::move(bytes));
        cursor += sizeof(T);
    }

    void add(Pad padding)
    {
        cursor += padding.amount;
    }

    void add(Capture capture)
    {
        captures.emplace_back(cursor, std::move(capture.name));
        cursor += capture.size;
    }

    void add(Bytes b)
    {
        auto size = b.data.size();
        parts.emplace_back(cursor, std::move(b.data));
        cursor += size;
    }

    std::size_t size()
    {
        return parts.back().offset + parts.back().bytes.size();
    }

    std::size_t following_size()
    {
        if (parts.size() < 2)
            return 0;

        return size() - parts[1].offset;
    }

    FindResult search(void* begin, void* end);

private:
    FindResult make_result(void* ptr);
};

template<class... Args>
Pattern make_pattern(Args&&... args)
{
    Pattern pat;
    auto dummy = {(pat.add(std::forward<Args>(args)), 0)...};
    return pat;
}

#endif // Header guard
