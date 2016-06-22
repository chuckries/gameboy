#include "stdafx.h"
#include "disassembler.h"
#include "memory.h"

std::string Disassembler::Instruction::GetFormattedCodeBytes()
{
    std::stringstream ss;
    ss << std::uppercase << std::hex;

    std::vector<u8>::iterator it = _codeBytes.begin();
    do
    {
        ss << std::setfill('0') << std::setw(2) << (i32)*it << ' ';
        it++;
    } while (it != _codeBytes.end());

    return ss.str();
}

std::string Disassembler::Instruction::GetDisassemblyString()
{
    return _ss.str();
}

void Disassembler::Instruction::Reset()
{
    _codeBytes.clear();
    _ss.clear();
}

Disassembler::Disassembler(std::shared_ptr<MemoryMap> mem)
    : _mem(mem)
    , _decode_alu{ "ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP " }
    , _decode_r{ "B", "C", "D", "E", "H", "L", "(HL)", "A" }
    , _decode_rp{ "BC", "DE", "HL", "SP" }
    , _decode_rp2{ "BC", "DE", "HL", "AF" }
    , _decode_cc{ "NZ", "Z", "NC", "C" }
{
}

Disassembler::~Disassembler()
{
}

void Disassembler::Disassemble(u16 pc, Disassembler::Instruction& instr)
{
    instr.Reset();
    _pc = pc;

    u8 op = Read8BumpPC();
    instr._codeBytes.push_back(op);

    bool prefix = op == 0xCB;
    if (prefix)
    {
        op = Read8BumpPC();
        instr._codeBytes.push_back(op);
    }

    u8 x = (op >> 6) & 0b11;
    u8 y = (op >> 3) & 0b111;
    u8 z = op & 0b111;
    u8 p = (op >> 4) & 0b11;
    u8 q = (op >> 3) & 0b1;

    if (!prefix)
    {
        switch (x)
        {
        case 0:
            {
                switch (z)
                {
                case 0:
                    {
                        switch (y)
                        {
                        case 0:
                            instr._ss << "NOP";
                            break;
                        case 1:
                        case 2:
                        case 3: __debugbreak();
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                            instr._codeBytes.push_back(Read8(_pc));
                            instr._ss << "JR " << _decode_cc[y - 4] << "," << DisplayBranchTarget();
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 1:
                    {
                        switch (q)
                        {
                        case 0:
                            // LD rp[p],nn
                            instr._codeBytes.push_back(Read8(_pc));
                            instr._codeBytes.push_back(Read8(_pc + 1));
                            instr._ss << "LD " << _decode_rp[p] << "," << Display16BumpPC();
                            break;
                        case 1:
                        default: __debugbreak();
                        }
                    }
                    break;
                case 2:
                    {
                        switch (q)
                        {
                        case 0:
                            {
                                switch (p)
                                {
                                case 0:
                                case 1:
                                case 2: __debugbreak();
                                case 3:
                                    instr._ss << "LDD (HL),A";
                                    break;
                                default: __debugbreak();
                                }
                            }
                            break;
                        case 1:
                            {
                                switch (p)
                                {
                                case 0:
                                case 1:
                                case 2: __debugbreak();
                                case 3:
                                    instr._ss << "LDD A,(HL)";
                                    break;
                                default: __debugbreak();
                                }
                            }
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 3:
                case 4: __debugbreak();
                case 5:
                    // DEC r[y]
                    instr._ss << "DEC " << _decode_r[y];
                    break;
                case 6:
                    // LD r[y],n
                    instr._codeBytes.push_back(Read8(_pc));
                    instr._ss << "LD " << _decode_r[y] << "," << Display8BumpPC();
                    break;
                case 7:
                default: __debugbreak();
                }
            }
            break;
        case 1: __debugbreak();
        case 2:
            // alu[y] r[z]
            instr._ss << _decode_alu[y] << _decode_r[z];
            break;
        case 3:
            {
                switch (z)
                {
                case 0:
                    {
                        switch (y)
                        {
                        case 0: // intentional fallthrough
                        case 1: // intentional fallthrough
                        case 2: // intentional fallthrough
                        case 3:
                            instr._ss << "RET " << _decode_cc[y];
                            break;
                        case 4:
                            instr._codeBytes.push_back(Read8(_pc));
                            instr._ss << "LD ($FF00+" << Display8BumpPC() << "),A";
                            break;
                        case 5:
                            instr._codeBytes.push_back(Read8(_pc));
                            instr._ss << "LD A,($FF00+" << Display8BumpPC() << ")";
                        case 6:
                        case 7:
                        default: __debugbreak();
                        }
                    }
                    break;
                case 1:
                case 2: __debugbreak();
                case 3:
                    {
                        switch (y)
                        {
                        case 0:
                            instr._codeBytes.push_back(Read8(_pc));
                            instr._codeBytes.push_back(Read8(_pc + 1));
                            instr._ss << "JP " << Display16BumpPC();
                            break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5: __debugbreak();
                        case 6:
                            instr._ss << "DI";
                            break;
                        case 7:
                            instr._ss << "EI";
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                default: __debugbreak();
                }
            }
            break;
        default: __debugbreak();
        }
    }
    else
    {
        switch (x)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        default: __debugbreak();
        }
    }
}

u8 Disassembler::Read8(u16 addr)
{
    return _mem->Load(addr);
}

u16 Disassembler::Read16(u16 addr)
{
    Pair word;
    word.W = 0;
    word.B.L = Read8(addr);
    word.B.H = Read8(addr + 1);
    return word.W;
}

u8 Disassembler::Read8BumpPC()
{
    return Read8(_pc++);
}

u16 Disassembler::Read16BumpPC()
{
    u16 word = Read16(_pc);
    _pc += 2;
    return word;
}

std::string Disassembler::Display8BumpPC()
{
    std::stringstream ss;
    ss << '$' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)Read8BumpPC();
    return ss.str();
}

std::string Disassembler::Display16BumpPC()
{
    std::stringstream ss;
    ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)Read16BumpPC();
    return ss.str();
}

u16 Disassembler::GetBranchTarget()
{
    i8 disp = (i8)Read8BumpPC();
    return (u16)((i32)_pc + (i32)disp);
}

std::string Disassembler::DisplayBranchTarget()
{
    std::stringstream ss;
    ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)GetBranchTarget();
    return ss.str();
}