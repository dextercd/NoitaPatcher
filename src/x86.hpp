#ifndef NP_X86_HPP
#define NP_X86_HPP

#include <cstdint>
#include <vector>
#include <iosfwd>

#include <Zydis/DecoderTypes.h>

#include "executable_info.hpp"

struct executable_info;

const std::uint8_t function_intro[]{
    0x55, 0x8b, 0xec
};

struct program_range {
    const void* start;
    const void* end;
};

struct function_range : program_range {};

const char* find_rdata_string(const executable_info& exe, const char* str);
function_range find_function_bounds(const executable_info& exe, const void* func_body);

// Given a pointer pointing into a loaded module, return the address based on
// the module's preferred load address.
std::uint32_t load_address(const executable_info& exe, const void* ptr);

void* real_address(const executable_info& exe, std::uint32_t loadaddr);

struct Instruction {
    ZydisDecodedInstruction inst;
    std::uint32_t operand_offset;
    std::uint32_t load_location;
};

struct DecodedRange {
    executable_info exe;
    const void* offset;
    std::uint32_t extent;

    std::vector<Instruction> instructions;
    std::vector<ZydisDecodedOperand> operands;

    using iterator = decltype(instructions)::iterator;
    using const_iterator = decltype(instructions)::const_iterator;

    iterator begin() { return instructions.begin(); }
    iterator end() { return instructions.end(); }
    const_iterator begin() const { return instructions.begin(); }
    const_iterator end() const { return instructions.end(); }
    const_iterator cbegin() const { return instructions.cbegin(); }
    const_iterator cend() const { return instructions.cend(); }

    const_iterator at_loadaddr(std::uint32_t loadaddr) const;
    iterator at_loadaddr(std::uint32_t loadaddr);


    std::uint32_t load_address() const
    {
        return ::load_address(exe, offset);
    }

    ZydisDecodedOperand* get_operands(const_iterator it)
    {
        return std::data(operands) + it->operand_offset;
    }

    const ZydisDecodedOperand* get_operands(const_iterator it) const
    {
        return std::data(operands) + it->operand_offset;
    }

private:
    iterator it_const_cast(const_iterator it)
    {
        return begin() + (it - cbegin());
    }
};

DecodedRange disassemble(const executable_info& exe, program_range range);
void dump_assembly(std::ostream& os, const DecodedRange& decoded);
void dump_instr(std::ostream& os, const DecodedRange& decoded, const Instruction& instr);

#endif // Header guard
