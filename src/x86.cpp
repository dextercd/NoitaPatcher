#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>

#include <Zydis/Decoder.h>
#include <Zydis/Formatter.h>

#include "executable_info.hpp"
#include "x86.hpp"

const char* find_rdata_string(const executable_info& exe, const char* str)
{
    auto found = std::search(
        exe.rdata_start, exe.rdata_end,
        (const std::uint8_t*)str, (const std::uint8_t*)str + std::strlen(str)
    );

    if (found == exe.rdata_end)
        return nullptr;

    return (const char*)found;
}

const void* find_function_start(const void* func_body)
{
    auto it = (const std::uint8_t*)func_body;
    for (;; --it) {
        if (
            ((std::uintptr_t)it % 16 == 0) &&        // Properly aligned
            (it[-1] == 0xcc ||                       // Alignment byte *or* Previous func return
                it[-1] == 0xc3 || it[-3] == 0xc2) &&
            (it[0] >= 0x50 && it[0] < 0x58) &&       // Push register
            (
                (it[1] >= 0x50 && it[1] < 0x58) ||   // Push register *or* mov ebp, esp
                (it[1] == 0x8b && it[2] == 0xec)
            )
        )
            return it;
    };
}

const void* find_function_end(const void* func_body)
{
    for (auto it = (const std::uint8_t*)func_body;; ++it) {
        if (
            (it[-1] >= 0x58 && it[-1] < 0x60) && // Pop register
            (it[0] == 0xc3 || it[0] == 0xc2)     // Ret (N)
        ) {
            auto next_instruction = it[0] == 0xc3 ? it + 1 : it + 3;
            return next_instruction;

            // Sometimes error handling/throwing is placed after the return, so
            // this "check for next function" logic doesn't work.

            /*
            // If this is really the return instruction, then the next function
            // should come right after this.  The function should be aligned to
            // a 16 byte boundary with 0 or more preceding padding bytes (0xcc).
            //
            // It doesn't make sense for there to be a padding byte on an
            // alignment point.
            for (
                auto seek_align = next_instruction;
                (*seek_align == 0xcc) != ((std::uintptr_t)seek_align % 16 == 0);
                ++seek_align
            ) {
                if ((std::uintptr_t)seek_align % 16 == 0)
                    return next_instruction; // Return one past the end pointer
            } */
        }
    }
}

function_range find_function_bounds(const executable_info& exe, const void* func_body)
{
    auto start = find_function_start(func_body);
    auto end = find_function_end(func_body);
    return function_range{start, end};
}

std::uint32_t load_address(const executable_info& exe, const void* ptr)
{
    if (!ptr)
        return 0;

    return (std::uint32_t)ptr - (std::uint32_t)exe.module + 0x400000u;
}

void* real_address(const executable_info& exe, std::uint32_t loadaddr)
{
    return (char*)exe.module + loadaddr - 0x400000u;
}

DecodedRange disassemble(const executable_info& exe, program_range range)
{
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

    auto load_addr = load_address(exe, range.start);
    auto offset = (const char*)range.start;
    const auto end = (const char*)range.end;

    DecodedRange decoded{exe, offset, (std::uint32_t)(end - offset), {}, {}};

    auto decode_next = [&]() {
        auto& instruction = decoded.instructions.emplace_back();
        instruction.operand_offset = decoded.operands.size();
        instruction.load_location = load_addr;

        // Allocate space for ZYDIS_MAX_OPERAND_COUNT operands
        auto const previous_operands_size = decoded.operands.size();
        decoded.operands.resize(previous_operands_size + ZYDIS_MAX_OPERAND_COUNT);
        auto operands = decoded.operands.data() + previous_operands_size;

        bool success = ZYAN_SUCCESS(
            ZydisDecoderDecodeFull(
                &decoder,
                offset,
                end - offset,
                &instruction.inst,
                operands));

        int operand_count = 0;
        if (success) {
            operand_count = instruction.inst.operand_count;
            load_addr += instruction.inst.length;
            offset += instruction.inst.length;
        } else {
            decoded.instructions.pop_back();
        }

        // Remove over provisioned operands
        decoded.operands.resize(previous_operands_size + operand_count);

        return success;
    };

    while (offset < end && decode_next());

    return decoded;
}

void dump_assembly(std::ostream& os, const DecodedRange& decoded)
{
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    auto os_flags = os.flags();
    auto os_fill = os.fill();

    os << std::hex << std::setfill('0');

    for (auto&& instruction : decoded.instructions) {
        auto operands = std::data(decoded.operands) + instruction.operand_offset;

        char buffer[256];
        ZydisFormatterFormatInstruction(&formatter, &instruction.inst, operands,
            instruction.inst.operand_count_visible, buffer, sizeof(buffer), instruction.load_location, ZYAN_NULL);

        os << std::setw(8) << instruction.load_location << "  " << buffer << '\n';
    }

    os.flags(os_flags);
    os.fill(os_fill);
}

void dump_instr(std::ostream& os, const DecodedRange& decoded, const Instruction& instruction)
{
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    auto os_flags = os.flags();
    auto os_fill = os.fill();

    os << std::hex << std::setfill('0');

    auto operands = std::data(decoded.operands) + instruction.operand_offset;

    char buffer[256];
    ZydisFormatterFormatInstruction(&formatter, &instruction.inst, operands,
        instruction.inst.operand_count_visible, buffer, sizeof(buffer), instruction.load_location, ZYAN_NULL);

    os << std::setw(8) << instruction.load_location << "  " << buffer << '\n';

    os.flags(os_flags);
    os.fill(os_fill);
}

DecodedRange::const_iterator DecodedRange::at_loadaddr(std::uint32_t loadaddr) const
{
    auto last = end();
    auto it = std::lower_bound(
        begin(), last, loadaddr,
        [] (const auto& instr, std::uint32_t load_addr) {
            return instr.load_location < load_addr;
        }
    );

    if (it != last && it->load_location == loadaddr)
        return it;

    return last;
}

DecodedRange::iterator DecodedRange::at_loadaddr(std::uint32_t loadaddr)
{
    return it_const_cast(std::as_const(*this).at_loadaddr(loadaddr));
}
