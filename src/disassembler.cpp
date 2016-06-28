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
    , _decode_rot{ "RLC", "RRC", "RL", "RR", "SLA", "SRA", "SWAP", "SRL" }
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
        switch (op)
        {
        case 0x00:
            // NOP
            instr._ss << "NOP";
            break;
        case 0x08:
            // LD (nn),SP
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "LD (" << Display16BumpPC() << "),SP";
            break;
        case 0x010:
            // STOP
            instr._codeBytes.push_back(Read8PC()); // 0x00 byte
            instr._ss << "STOP";
            break;
        case 0x18:
            // JR d
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "JR " << DisplayBranchTarget();
            break;
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
            // JR cc[y-4],d
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "JR " << _decode_cc[y - 4] << "," << DisplayBranchTarget();
        break;
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
            // LD rp[p],nn
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "LD " << _decode_rp[p] << "," << Display16BumpPC();
            break;
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
            // ADD HL,rp[p]
            instr._ss << "ADD HL," << _decode_rp[p];
            break;
        case 0x02:
            // LD (BC),A
            instr._ss << "LD (BC),A";
            break;
        case 0x12:
            // LD (DE),A
            instr._ss << "LD (DE),A";
            break;
        case 0x22:
            // LDI (HL),A
            instr._ss << "LDI (HL),A";
            break;
        case 0x32:
            /// LDD (HL),A
            instr._ss << "LDD (HL),A";
            break;
        case 0x0A:
            // LD A,(BC)
            instr._ss << "LD A,(BC)";
            break;
        case 0x1A:
            // LD A,(DE)
            instr._ss << "LD A,(DE)";
            break;
        case 0x2A:
            // LDI A,(HL)
            instr._ss << "LDI A,(HL)";
            break;
        case 0x3A:
            // LDD A,(HL)
            instr._ss << "LDD A,(HL)";
            break;
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
            // INC rp[p]
            instr._ss << "INC " << _decode_rp[p];
            break;
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
            // DEC rp[p]
            instr._ss << "DEC " << _decode_rp[p];
            break;
        case 0x04:
        case 0x0C:
        case 0x14:
        case 0x1C:
        case 0x24:
        case 0x2C:
        case 0x34:
        case 0x3C:
            // INC r[y]
            instr._ss << "INC " << _decode_r[y];
            break;
        case 0x05:
        case 0x0D:
        case 0x15:
        case 0x1D:
        case 0x25:
        case 0x2D:
        case 0x35:
        case 0x3D:
            // DEC r[y]
            instr._ss << "DEC " << _decode_r[y];
            break;
        case 0x06:
        case 0x0E:
        case 0x16:
        case 0x1E:
        case 0x26:
        case 0x2E:
        case 0x36:
        case 0x3E:
            // LD r[y],n
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "LD " << _decode_r[y] << "," << Display8BumpPC();
            break;
        case 0x07:
            // RLCA
            instr._ss << "RLCA";
            break;
        case 0xF:
            // RRCA
            instr._ss << "RRCA";
            break;
        case 0x17:
            // RLA
            instr._ss << "RLA";
            break;
        case 0x1F:
            // RRA
            instr._ss << "RRA";
            break;
        case 0x27:
            // DAA
            instr._ss << "DAA";
            break;
        case 0x2F:
            // CPL
            instr._ss << "CPL";
            break;
        case 0x37:
            // SCF
            instr._ss << "SCF";
            break;
        case 0x3F:
            // CCF
            instr._ss << "CCF";
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
            // case 0x76: HALT
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
            // LD r[y],r[z]
            instr._ss << "LD " << _decode_r[y] << "," << _decode_r[z];
            break;
        case 0x76:
            // HALT
            instr._ss << "HALT";
            break;
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            // alu[y] r[z]
            instr._ss << _decode_alu[y] << _decode_r[z];
            break;
        case 0xC0:
        case 0xC8:
        case 0xD0:
        case 0xD8:
            // RET cc[y]
            instr._ss << "RET " << _decode_cc[y];
        break;
        case 0xE0:
            // LD ($FF00+n),A
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "LD ($FF00+" << Display8BumpPC() << "),A";
            break;
        case 0xE8:
            // ADD SP,n
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "ADD SP," << Display8BumpPC();
            break;
        case 0xF0:
            // LD A,($FF00+n)
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "LD A,($FF00+" << Display8BumpPC() << ")";
            break;
        case 0xF8:
            // LDHL
            instr._codeBytes.push_back(Read8PC());
            instr._ss << "LDHL SP," << Display8BumpPC();
            break;
        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
            // POP rp2[p]
            instr._ss << "POP " << _decode_rp2[p];
            break;
        case 0xC9:
            // RET
            instr._ss << "RET";
            break;
        case 0xD9:
            // RETI
            instr._ss << "RETI";
            break;
        case 0xE9:
            // JP HL
            instr._ss << "JP HL";
            break;
        case 0xF9:
            // LD SP,HL
            instr._ss << "LD SP,HL";
            break;
        case 0xC2:
        case 0xCA:
        case 0xD2:
        case 0xDA:
            // JP cc[y],nn
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "JP " << _decode_cc[y] << "," << Display16BumpPC();
            break;
        case 0xE2:
            // LD ($FF00+C),A
            instr._ss << "LD ($FF00+C),A";
            break;
        case 0xEA:
            // LD (nn),A
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "LD (" << Display16BumpPC() << "),A";
            break;
        case 0xF2:
            // LD A,($FF00+C)
            instr._ss << "LD A,($FF00+C)";
            break;
        case 0xFA:
            // LD A,(nn)
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "LD A,(" << Display16BumpPC() << ")";
            break;
        case 0xC3:
            // JP nn
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "JP " << Display16BumpPC();
            break;
        case 0xF3:
            // DI
            instr._ss << "DI";
            break;
        case 0xFB:
            // EI
            instr._ss << "EI";
            break;
        case 0xC4:
        case 0xCC:
        case 0xD4:
        case 0xDC:
            // CALL cc[y],nn
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "CALL " << _decode_cc[y] << "," << Display16BumpPC();
            break;
        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
            // PUSH rp2[p]
            instr._ss << "PUSH " << _decode_rp2[p];
            break;
        case 0xCD:
            // CALL nn
            instr._codeBytes.push_back(Read8(_pc));
            instr._codeBytes.push_back(Read8(_pc + 1));
            instr._ss << "CALL " << Display16BumpPC();
            break;
        case 0xC6:
        case 0xCE:
        case 0xD6:
        case 0xDE:
        case 0xE6:
        case 0xEE:
        case 0xF6:
        case 0xFE:
            // alu[y] n
            instr._codeBytes.push_back(Read8PC());
            instr._ss << _decode_alu[y] << Display8BumpPC();
            break;
        case 0xC7:
        case 0xCF:
        case 0xD7:
        case 0xDF:
        case 0xE7:
        case 0xEF:
        case 0xF7:
        case 0xFF:
            // RST y*8
            instr._ss << "RST " << '$' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (y << 3);
            break;
        default:
            __debugbreak();
            break;
        }
    }
    else
    {
        switch (op & 0xC0)
        {
        case 0x00:
            // rot[y] r[z]
            instr._ss << _decode_rot[y] << " " << _decode_r[z];
            break;
        case 0x40:
            // BIT y,r[z]
            instr._ss << "BIT" << y << "," <<_decode_r[z];
            break;
        case 0x80:
            // RES y,r[z]
            instr._ss << "RES" << y << "," << _decode_r[z];
            break;
        case 0xC0:
            // SET y,r[z]
            instr._ss << "SET" << y << "," << _decode_r[z];
            break;
        default:
            __debugbreak();
            break;
        }
    }
}

u8 Disassembler::Read8(u16 addr)
{
    return _mem->Load(addr);
}

u16 Disassembler::Read16(u16 addr)
{
    Word word;
    word._0 = Read8(addr);
    word._1 = Read8(addr + 1);
    return *word;
}

u8 Disassembler::Read8PC()
{
    return Read8(_pc);
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