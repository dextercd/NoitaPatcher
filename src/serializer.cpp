#include <cstdint>
#include <cstring>
#include <string>

#include <vs2013/string.hpp>

#include "serializer.hpp"

Serializer* SerialSaver::destructor(bool delet)
{
    if (delet) {
        delete this;
    } else {
        this->~SerialSaver();
    }

    return this;
}

bool SerialSaver::IsSaving() const { return true; }
bool SerialSaver::HasOverflowed() const { return false; }

std::uint8_t* SerialSaver::write_space(std::size_t length)
{
    auto write_offset = buffer.size();
    buffer.resize(buffer.size() + length);
    return (std::uint8_t*)(buffer.data() + write_offset);
}

void SerialSaver::IO(vs13::string& value)
{
    auto sz = (std::uint32_t)value.size();
    IO(sz);

    std::memcpy(write_space(value.size()), value.c_str(), value.size());
}

void SerialSaver::IO(bool& value)
{
    auto w = (std::uint8_t)value;
    IO(w);
}

void SerialSaver::IO(double& value)
{
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    std::uint64_t to_write;
    std::memcpy(&to_write, &value, sizeof(to_write));
    IO(to_write);
}

void SerialSaver::IO(float& value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    std::uint32_t to_write;
    std::memcpy(&to_write, &value, sizeof(to_write));
    IO(to_write);
}

void SerialSaver::IO(std::int64_t& value)
{
    auto w = (std::uint64_t)value;
    IO(w);
}

void SerialSaver::IO(std::uint64_t& value)
{
    auto w = write_space(8);
    w[0] = value >> 56;
    w[1] = value >> 48;
    w[2] = value >> 40;
    w[3] = value >> 32;
    w[4] = value >> 24;
    w[5] = value >> 16;
    w[6] = value >> 8;
    w[7] = value;
}

void SerialSaver::IO(std::int32_t& value)
{
    auto w = (std::uint32_t)value;
    IO(w);
}

void SerialSaver::IO(std::uint32_t& value)
{
    auto w = write_space(4);
    w[0] = value >> 24;
    w[1] = value >> 16;
    w[2] = value >> 8;
    w[3] = value;
}

void SerialSaver::IO(std::int16_t& value)
{
    auto w = (std::uint16_t)value;
    IO(w);
}

void SerialSaver::IO(std::uint16_t& value)
{
    auto w = write_space(2);
    w[0] = value >> 8;
    w[1] = value;
}

void SerialSaver::IO(std::int8_t& value)
{
    auto w = (std::uint8_t)value;
    IO(w);
}

void SerialSaver::IO(std::uint8_t& value)
{
    write_space(1)[0] = value;
}


// SerialLoader

Serializer* SerialLoader::destructor(bool delet)
{
    if (delet) {
        delete this;
    } else {
        this->~SerialLoader();
    }

    return this;
}

bool SerialLoader::IsSaving() const { return false; }
bool SerialLoader::HasOverflowed() const { return overflowed; }

std::uint8_t* SerialLoader::read_space(std::size_t length)
{
    if (current_it + length < data_end) {
        auto read_ptr = current_it;
        current_it += length;
        return (std::uint8_t*)read_ptr;
    }

    overflowed = true;
    current_it = data_end;
    return nullptr;
}

void SerialLoader::IO(vs13::string& value)
{
    std::uint32_t size = -1;
    IO(size);

    auto read = read_space(size);
    if (!read)
        return;

    value.resize(size);
    std::memcpy(value.data(), read, value.size());
}

void SerialLoader::IO(bool& value)
{
    auto read = read_space(1);
    if (!read)
        return;

    value = read[0] != 0;
}

void SerialLoader::IO(double& value)
{
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    std::uint64_t to_read;
    IO(to_read);

    if (!HasOverflowed())
        std::memcpy(&value, &to_read, sizeof(value));
}

void SerialLoader::IO(float& value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    std::uint32_t to_read;
    IO(to_read);

    if (!HasOverflowed())
        std::memcpy(&value, &to_read, sizeof(value));
}

void SerialLoader::IO(std::int64_t& value)
{
    IO((std::uint64_t&)value);
}

void SerialLoader::IO(std::uint64_t& value)
{
    auto r = read_space(8);
    if (!r)
        return;

    value = (std::uint64_t)r[0] << 56
          | (std::uint64_t)r[1] << 48
          | (std::uint64_t)r[2] << 40
          | (std::uint64_t)r[3] << 32
          | (std::uint64_t)r[4] << 24
          | (std::uint64_t)r[5] << 16
          | (std::uint64_t)r[6] << 8
          | (std::uint64_t)r[7];
}

void SerialLoader::IO(std::int32_t& value)
{
    IO((std::uint32_t&)value);
}

void SerialLoader::IO(std::uint32_t& value)
{
    auto r = read_space(4);
    if (!r)
        return;

    value = (std::uint32_t)r[0] << 24
          | (std::uint32_t)r[1] << 16
          | (std::uint32_t)r[2] << 8
          | (std::uint32_t)r[3];
}

void SerialLoader::IO(std::int16_t& value)
{
    IO((std::uint16_t&)value);
}

void SerialLoader::IO(std::uint16_t& value)
{
    auto r = read_space(2);
    if (!r)
        return;

    value = (std::uint16_t)r[0] << 8
          | (std::uint16_t)r[1];
}

void SerialLoader::IO(std::int8_t& value)
{
    IO((std::uint8_t&)value);
}

void SerialLoader::IO(std::uint8_t& value)
{
    auto r = read_space(1);
    if (!r)
        return;

    value = r[0];
}
