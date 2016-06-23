#include "stdafx.h"
#include "cpu.h"
#include "memory.h"
#include "disassembler.h"

const u8 Cpu::Z_FLAG = (1 << 7);
const u8 Cpu::N_FLAG = (1 << 6);
const u8 Cpu::H_FLAG = (1 << 5);
const u8 Cpu::C_FLAG = (1 << 4);

Cpu::Cpu(std::shared_ptr<MemoryMap> mem)
    : _mem(mem)
    , _disassembler(std::make_unique<Disassembler>(mem))
    , _interrupt_ime(false)
    , _interrupt_ime_lag(false)
    , _regs{ 0 }
    , _regA(_regs.AF.B.A)
    , _regB(_regs.BC.B.B)
    , _regC(_regs.BC.B.C)
    , _regD(_regs.DE.B.D)
    , _regE(_regs.DE.B.E)
    , _regF(_regs.AF.B.F)
    , _regH(_regs.HL.B.H)
    , _regL(_regs.HL.B.L)
    , _regSP(_SP.W)
    , _regAF(_regs.AF.W)
    , _regBC(_regs.BC.W)
    , _regDE(_regs.DE.W)
    , _regHL(_regs.HL.W)
    , _indBC(*this, _regs.BC.W)
    , _indDE(*this, _regs.DE.W)
    , _indHL(*this, _regs.HL.W)
    , _indImm(*this)
    , _imm8(*this)
    , _imm16(*this)
    , _decode_r{ &_regB, &_regC, &_regD, &_regE, &_regH, &_regL, &_indHL, &_regA }
    , _decode_rp{ &_regBC, &_regDE, &_regHL, &_regSP }
    , _decode_rp2{ &_regBC, &_regDE, &_regHL, &_regAF }
    , _decode_alu{ &Cpu::ADD8, &Cpu::ADC, &Cpu::SUB, &Cpu::SBC, &Cpu::AND, &Cpu::XOR, &Cpu::OR, &Cpu::CP }
    , _decode_cc{ &Cpu::CondNZ, &Cpu::CondZ, &Cpu::CondNC, &Cpu::CondC }
{

}

Cpu::~Cpu()
{

}

void Cpu::Init()
{
    _PC.W = 0x100;
    _regs.AF.W = 0x01B0;
    _regs.BC.W = 0x0013;
    _regs.DE.W = 0x00D8;
    _regs.HL.W = 0x014D;
    _SP.W = 0xFFFE;
}

u8 Cpu::Read8(u16 addr)
{
    return _mem->Load(addr);
}

u16 Cpu::Read16(u16 addr)
{
    Pair word;
    word.B.L = Read8(addr);
    word.B.H = Read8(addr + 1);
    return word.W;
}

void Cpu::Write8(u16 addr, u8 val)
{
    _mem->Store(addr, val);
}

void Cpu::Write16(u16 addr, u16 val)
{
    Pair word;
    word.W = val;
    Write8(addr, word.B.L);
    Write8(addr + 1, word.B.H);
}

void Cpu::Step()
{
    DoInterrupt();
    Trace();
    Decode();
    CleanState();
}

void Cpu::CleanState()
{
    _pOpSrc8 = nullptr;
    _pOpDst8 = nullptr;
    _pOpSrc16 = nullptr;
    _pOpDst16 = nullptr;
}

void Cpu::DoInterrupt()
{
    if (_interrupt_ime_lag)
    {
        __debugbreak();
    }
    else if (_interrupt_ime)
    {
        _interrupt_ime_lag = true;
    }
}

void Cpu::Decode()
{
    u8 op = Read8BumpPC();

    bool prefix = op == 0xCB;
    if (prefix)
    {
        op = Read8BumpPC();
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
            break;
        case 0x010:
            // DJNZ
            __debugbreak();
            break;
        case 0x18:
            // JR d
            JR(true);
            break;
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
            // JR cc[y-4],d
            JR((*this.*_decode_cc[y - 4])());
            break;
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
            // LD rp[p],nn
            _pOpSrc16 = &_imm16.FromPC();
            _pOpDst16 = _decode_rp[p];
            LD16();
            break;
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
            // ADD HL,rp[p]
            _pOpSrc16 = _decode_rp[p];
            _pOpDst16 = &_regHL;
            ADD16();
            break;
        case 0x02:
            // LD (BC),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indBC;
            LD8();
            break;
        case 0x12:
            // LD (DE),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indDE;
            LD8();
            break;
        case 0x22:
            // LDI (HL),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indHL;
            LD8();
            _regs.HL.W++;
            break;
        case 0x32:
            /// LDD (HL),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indHL;
            LD8();
            _regs.HL.W--;
            break;
        case 0x0A:
            // LD A,(BC)
            _pOpSrc8 = &_indBC;
            _pOpDst8 = &_regA;
            LD8();
            break;
        case 0x1A:
            // LD A,(DE)
            _pOpSrc8 = &_indDE;
            _pOpDst8 = &_regA;
            LD8();
            break;
        case 0x2A:
            // LDI A,(HL)
            _pOpSrc8 = &_indHL;
            _pOpSrc8 = &_regA;
            LD8();
            _regs.HL.W++;
            break;
        case 0x3A:
            // LDD A,(HL)
            _pOpSrc8 = &_indHL;
            _pOpDst8 = &_regA;
            LD8();
            _regs.HL.W--;
            break;
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
            // INC rp[p]
            _pOpSrc16 = _decode_rp[p];
            INC16();
            break;
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
            // DEC rp[p]
            _pOpSrc16 = _decode_rp[p];
            DEC16();
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
            _pOpSrc8 = _decode_r[y];
            INC8();
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
            _pOpSrc8 = _decode_r[y];
            DEC8();
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
            _pOpSrc8 = &_imm8.FromPC();
            _pOpDst8 = _decode_r[y];
            LD8();
            break;
        case 0x07:
            // RLCA
            __debugbreak();
            break;
        case 0xF:
            // RRCA
            __debugbreak();
            break;
        case 0x17:
            // RLA
            __debugbreak();
            break;
        case 0x1F:
            // RRA
            __debugbreak();
            break;
        case 0x27:
            // DAA
            __debugbreak();
            break;
        case 0x2F:
            // CPL
            __debugbreak();
            break;
        case 0x37:
            // SCF
            SetC(true);
            ResetN();
            ResetH();
            break;
        case 0x3F:
            // CCF
            SetC(false);
            ResetN();
            ResetH();
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
            _pOpSrc8 = _decode_r[z];
            _pOpDst8 = _decode_r[y];
            LD8();
            break;
        case 0x76:
            // HALT
            __debugbreak();
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
            _pOpSrc8 = _decode_r[z];
            (*this.*_decode_alu[y])();
            break;
        case 0xC0:
        case 0xC8:
        case 0xD0:
        case 0xD8:
            // RET cc[y]
            RET((*this.*_decode_cc[y])());
            break;
        case 0xE0:
            // LD ($FF00+n),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indImm.ForLDH();
            LD8();
            break;
        case 0xE8:
            // ADD SP,n
            __debugbreak();
            break;
        case 0xF0:
            // LD A,($FF00+n)
            _pOpSrc8 = &_indImm.ForLDH();
            _pOpDst8 = &_regA;
            LD8();
            break;
        case 0xF8:
            // LDHL
            __debugbreak();
            break;
        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
            // POP rp2[p]
            __debugbreak();
            break;
        case 0xC9:
            // RET
            RET(true);
            break;
        case 0xD9:
            // RETI
            RETI();
            break;
        case 0xE9:
            // JP HL
            __debugbreak();
            break;
        case 0xF9:
            // LD SP,HL
            _pOpSrc16 = &_regHL;
            _pOpDst16 = &_regSP;
            LD16();
            break;
        case 0xC2:
        case 0xCA:
        case 0xD2:
        case 0xDA:
            // JP cc[y],nn
            _pOpSrc16 = &_imm16.FromPC();
            JP((*this.*_decode_cc[y])());
            break;
        case 0xE2:
            // LD ($FF00+C),A
            __debugbreak();
            break;
        case 0xEA:
            // LD (nn),A
            __debugbreak();
            break;
        case 0xF2:
            // LD A,($FF00+C)
            __debugbreak();
            break;
        case 0xFA:
            // LD A,(nn)
            __debugbreak();
            break;
        case 0xC3:
            // JP nn
            _pOpSrc16 = &_imm16.FromPC();
            JP(true);
            break;
        case 0xF3:
            // DI
            DI();
            break;
        case 0xFB:
            // EI
            EI();
            break;
        case 0xC4:
        case 0xCC:
        case 0xD4:
        case 0xDC:
            // CALL cc[y],nn
            __debugbreak();
            break;
        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
            // PUSH rp2[p]
            __debugbreak();
            break;
        case 0xCD:
            // CALL nn
            __debugbreak();
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
            _pOpSrc8 = &_imm8.FromPC();
            (*this.*_decode_alu[y])();
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
            __debugbreak();
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
            __debugbreak();
            break;
        case 0x40:
            // BIT y,r[z]
            _pOpSrc8 = _decode_r[z];
            BIT(y);
            break;
        case 0x80:
            // RES y,r[z]
            _pOpSrc8 = _decode_r[z];
            RES(y);
            break;
        case 0xC0:
            // SET y,r[z]
            _pOpSrc8 = _decode_r[z];
            SET(y);
            break;
        default:
            __debugbreak();
            break;
        }
    }
}

u8 Cpu::Read8BumpPC()
{
    return Read8(_PC.W++);
}

u16 Cpu::Read16BumpPC()
{
    u16 word = Read16(_PC.W);
    _PC.W += 2;
    return word;
}

// Flag Operations
void Cpu::ResetFlags()
{
    _regs.AF.B.F = 0;
}

void Cpu::SetFlag(u8 flag, bool set)
{
    if (set)
    {
        _regs.AF.B.F |= flag;
    }
    else
    {
        _regs.AF.B.F &= ~flag;
    }
}

void Cpu::SetZ(u8 val)
{
    SetFlag(Z_FLAG, val == 0);
}

void Cpu::SetN()
{
    SetFlag(N_FLAG, true);
}

void Cpu::ResetN()
{
    SetFlag(N_FLAG, false);
}

void Cpu::SetH(bool set)
{
    SetFlag(H_FLAG, set);
}

void Cpu::ResetH()
{
    SetH(false);
}

void Cpu::SetC(bool set)
{
    SetFlag(C_FLAG, set);
}

bool Cpu::CondFlag(u8 flag)
{
    return (_regs.AF.B.F & flag) != 0;
}

bool Cpu::CondNZ()
{
    return !CondFlag(Z_FLAG);
}

bool Cpu::CondZ()
{
    return CondFlag(Z_FLAG);
}

bool Cpu::CondNC()
{
    return !CondFlag(C_FLAG);
}

bool Cpu::CondC()
{
    return CondFlag(C_FLAG);
}

///*
/// Operations
///*

void Cpu::LD8()
{
    _pOpDst8->Write(_pOpSrc8->Read());
}

void Cpu::LD16()
{
    _pOpDst16->Write(_pOpSrc16->Read());
}

void Cpu::LDHL()
{
    __debugbreak();
}

void Cpu::ADD8()
{
    __debugbreak();
}

void Cpu::ADD16()
{
    __debugbreak();
}

void Cpu::ADC()
{
    __debugbreak();
}

void Cpu::SUB()
{
    _regs.AF.B.A = sub_help();
}

void Cpu::SBC()
{
    __debugbreak();
}

void Cpu::AND()
{
    __debugbreak();
}

void Cpu::XOR()
{
    _regs.AF.B.A ^= _pOpSrc8->Read();
    ResetFlags();
    SetZ(_regs.AF.B.A);
}

void Cpu::OR()
{
    __debugbreak();
}

void Cpu::CP()
{
    sub_help();
}

void Cpu::INC8()
{
    u8 val = _pOpSrc8->Read();
    u8 newVal = val + 1;
    _pOpSrc8->Write(newVal);
    SetZ(newVal);
    ResetN();
    SetH(false);
    val &= 0x0F;
    val++;
    SetH((val & 0xF0) != 0);
}

void Cpu::INC16()
{
    __debugbreak();
}

void Cpu::DEC8()
{
    u8 val = _pOpSrc8->Read();
    u8 newVal = val - 1;
    _pOpSrc8->Write(newVal);
    SetZ(newVal);
    SetN();
    SetH(false);
    val = (val & 0x0F);
    val--;
    SetH((val & 0xF0) != 0);
}

void Cpu::DEC16()
{
    __debugbreak();
}

void Cpu::SWAP()
{
    __debugbreak();
}

void Cpu::DAA()
{
    __debugbreak();
}

void Cpu::CPL()
{
    __debugbreak();
}

void Cpu::HALT()
{
    __debugbreak();
}

void Cpu::STOP()
{
    __debugbreak();
}

void Cpu::DI()
{
    _interrupt_ime = false;
    _interrupt_ime_lag = false;
}

void Cpu::EI()
{
    _interrupt_ime = true;
}

void Cpu::RLCA()
{
    __debugbreak();
}

void Cpu::RLA()
{
    __debugbreak();
}

void Cpu::RRCA()
{
    __debugbreak();
}

void Cpu::RRA()
{
    __debugbreak();
}

void Cpu::RLC()
{
    __debugbreak();
}

void Cpu::RL()
{
    __debugbreak();
}

void Cpu::RRC()
{
    __debugbreak();
}

void Cpu::RR()
{
    __debugbreak();
}

void Cpu::SLA()
{
    __debugbreak();
}

void Cpu::SRA()
{
    __debugbreak();
}

void Cpu::SRL()
{
    __debugbreak();
}

void Cpu::BIT(u8 val)
{
    __debugbreak();
}

void Cpu::SET(u8 val)
{
    __debugbreak();
}

void Cpu::RES(u8 val)
{
    __debugbreak();
}

void Cpu::JP(bool condition)
{
    u16 newPC = _pOpSrc16->Read();
    if (condition)
    {
        _PC.W = newPC;
    }
}

void Cpu::JR(bool condition)
{
    i8 disp = (i8)Read8BumpPC();
    if (condition)
    {
        _PC.W += (i16)disp;
    }
}

void Cpu::CALL(bool condition)
{
    __debugbreak();
}

void Cpu::RST(u8 val)
{
    __debugbreak();
}

void Cpu::RET(bool cond)
{
    __debugbreak();
}

void Cpu::RETI()
{
    __debugbreak();
}

u8 Cpu::sub_help()
{
    Pair word;
    u8 left = _regs.AF.B.A;
    u8 right = _pOpSrc8->Read();

    word.W = left - right;
    SetZ(word.B.L);
    SetN();
    SetC(word.B.H != 0);

    left &= 0x0F;
    right &= 0x0F;
    left -= right;
    SetH((left & 0xF0) != 0);

    return word.B.L;
}

#if defined(TRACE)
void Cpu::Trace()
{
    Disassembler::Instruction instr;
    _disassembler->Disassemble(_PC.W, instr);

    printf("%04X %-14s %-18s A:%02X B:%02X C:%02X D:%02X E:%02X F:%02X H:%02X L:%02X AF:%04X BC:%04X DE:%04X HL:%04X SP:%04X\n",
        _PC.W,
        instr.GetFormattedCodeBytes().c_str(),
        instr.GetDisassemblyString().c_str(),
        _regs.AF.B.A,
        _regs.BC.B.B,
        _regs.BC.B.C,
        _regs.DE.B.D,
        _regs.DE.B.E,
        _regs.AF.B.F,
        _regs.HL.B.H,
        _regs.HL.B.L,
        _regs.AF.W,
        _regs.BC.W,
        _regs.DE.W,
        _regs.HL.W,
        _SP.W
        );
}
#else
void Cpu::Trace() {}
#endif