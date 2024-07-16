#ifndef NP_VS13_STRING_HPP
#define NP_VS13_STRING_HPP

#include <cstdint>
#include <cstring>
#include <string_view>
#include <utility>

#include <vs2013/memory.hpp>

namespace vs13 {

struct string {
    char* buffer = nullptr;
    char sso_buffer[12]{};
    std::size_t size_ = 0;
    std::size_t capacity_ = 15;

    string() = default;

    string(const char* str, std::size_t length)
    {
        if (length > 15) {
            buffer = (char*)vs13::operator_new(length + 1);
            capacity_ = length;
        }

        std::memcpy(data(), str, length);
        data()[length] = '\0';
        size_ = length;
    }

    string& operator=(const string& other) noexcept
    {
        if (other.size_ <= capacity_) {
            std::memcpy(data(), other.data(), other.size_);
            size_ = other.size_;
            return *this;
        }

        deallocate();
        *this = string{other.data(), other.size_};
        return *this;
    }

    string& operator=(string&& other) noexcept
    {
        std::swap(buffer, other.buffer);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        other.deallocate();
        return *this;
    }

    void resize(std::size_t new_size)
    {
        auto old_size = size();
        auto old_capacity = capacity();

        if (old_size >= new_size) {
            // Reducing in size
            size_ = new_size;
            data()[new_size] = '\0';
            return;
        }

        if (old_capacity >= new_size) {
            // Increasing in size but not capacity
            std::memset(data() + old_size, 0, new_size - old_size + 1);
            size_ = new_size;
            return;
        }

        // Increasing in capacity and size
        auto new_buffer = (char*)vs13::operator_new(new_size + 1);
        std::memcpy(new_buffer, data(), old_size);
        std::memset(new_buffer + old_size, 0, new_size - old_size + 1);

        if (!is_sso())
            vs13::operator_delete(buffer);

        buffer = new_buffer;
        size_ = new_size;
        capacity_ = new_size;
    }

    ~string()
    {
        deallocate();
    }

    void deallocate()
    {
        if (!is_sso()) {
            vs13::operator_delete(buffer);
            size_ = 0;
            capacity_ = 15;
        }
    }

    const char* c_str() const
    {
        return data();
    }

    char* data() { return (char*)(std::as_const(*this).data()); }

    const char* data() const
    {
        if (is_sso())
            return (const char*)&buffer;

        return buffer;
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }

    std::string_view as_view() const { return {c_str(), size()}; }

    char& operator[](std::size_t i) { return data()[i]; }
    const char& operator[](std::size_t i) const { return data()[i]; }

private:
    bool is_sso() const
    {
        return capacity() < 0x10;
    }
};

} // vs13::

#endif // Header guard
