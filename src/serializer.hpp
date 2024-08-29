#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <cstdint>
#include <cstring>
#include <string>

#include <vs2013/string.hpp>

// Class whose layout is compatible with Poro's ISerializer class.
// The input/output generated is not the same as that of Poro.
class Serializer {
public:
    virtual Serializer* destructor(bool delet) = 0;        // 00
    virtual bool IsSaving() const = 0;                     // 04
    virtual bool IsLoading() const { return !IsSaving(); } // 08
    virtual bool HasOverflowed() const = 0;                // 0c
    virtual void sIO(vs13::string& value) = 0;             // 10
    virtual void bIO(bool& value) = 0;                     // 14
    virtual void dIO(double& value) = 0;                   // 18
    virtual void fIO(float& value) = 0;                    // 1c
    virtual void i64IO(std::int64_t& value) = 0;           // 20
    virtual void u64IO(std::uint64_t& value) = 0;          // 24
    virtual void i32IO(std::int32_t& value) = 0;           // 28
    virtual void u32IO(std::uint32_t& value) = 0;          // 2c
    virtual void i16IO(std::int16_t& value) = 0;           // 30
    virtual void u16IO(std::uint16_t& value) = 0;          // 34
    virtual void i8IO(std::int8_t& value) = 0;             // 38
    virtual void u8IO(std::uint8_t& value) = 0;            // 3c
};

class SerialSaver final : public Serializer {
private:
    std::uint8_t* write_space(std::size_t length);

public:
    std::string buffer;

    Serializer* destructor(bool delet) override;
    bool IsSaving() const override;
    bool IsLoading() const { return !IsSaving(); }
    bool HasOverflowed() const override;
    void sIO(vs13::string& value) override;
    void bIO(bool& value) override;
    void dIO(double& value) override;
    void fIO(float& value) override;
    void i64IO(std::int64_t& value) override;
    void u64IO(std::uint64_t& value) override;
    void i32IO(std::int32_t& value) override;
    void u32IO(std::uint32_t& value) override;
    void i16IO(std::int16_t& value) override;
    void u16IO(std::uint16_t& value) override;
    void i8IO(std::int8_t& value) override;
    void u8IO(std::uint8_t& value) override;
};

class SerialLoader final : public Serializer {
    const char* data_start;
    const char* data_end;
    const char* current_it;

    bool overflowed = false;

    std::uint8_t* read_space(std::size_t length);

public:
    SerialLoader(const char* data, std::size_t data_sz)
        : data_start{data}
        , data_end{data_start + data_sz}
        , current_it{data_start}
    {
    }

    Serializer* destructor(bool delet) override;
    bool IsSaving() const override;
    bool IsLoading() const { return !IsSaving(); }
    bool HasOverflowed() const override;
    void sIO(vs13::string& value) override;
    void bIO(bool& value) override;
    void dIO(double& value) override;
    void fIO(float& value) override;
    void i64IO(std::int64_t& value) override;
    void u64IO(std::uint64_t& value) override;
    void i32IO(std::int32_t& value) override;
    void u32IO(std::uint32_t& value) override;
    void i16IO(std::int16_t& value) override;
    void u16IO(std::uint16_t& value) override;
    void i8IO(std::int8_t& value) override;
    void u8IO(std::uint8_t& value) override;
};

#endif // Header guard
