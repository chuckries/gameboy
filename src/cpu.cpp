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
    , _decode_alu{ &Cpu::ADD, &Cpu::ADC, &Cpu::SUB, &Cpu::SBC, &Cpu::AND, &Cpu::XOR, &Cpu::OR, &Cpu::CP }
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
                            // NOP
                            break;
                        case 1: __debugbreak();
                        case 2: __debugbreak();
                        case 3: __debugbreak();
                        case 4: // intentional fallthrough
                        case 5: // intentional fallthrough
                        case 6: // intentional fallthrough
                        case 7:
                            JR((*this.*_decode_cc[y - 4])());
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
                            _pOpSrc16 = &_imm16.FromPC();
                            _pOpDst16 = _decode_rp[p];
                            LD16();
                            break;
                        case 1: __debugbreak();
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
                                case 0: __debugbreak();
                                case 1: __debugbreak();
                                case 2: __debugbreak();
                                case 3:
                                    /// LDD (HL),A
                                    _pOpSrc8 = &_regA;
                                    _pOpDst8 = &_indHL;
                                    LD8();
                                    _regs.HL.W--;
                                    break;
                                default: __debugbreak();
                                }
                            }
                            break;
                        case 1:
                            {
                                switch (p)
                                {
                                case 0: __debugbreak();
                                case 1: __debugbreak();
                                case 2: __debugbreak();
                                case 3:
                                    /// LDD A,(HL)
                                    _pOpSrc8 = &_indHL;
                                    _pOpDst8 = &_regA;
                                    LD8();
                                    _regs.HL.W--;
                                    break;
                                default: __debugbreak();
                                }
                            }
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 3: __debugbreak();
                case 4: __debugbreak();
                case 5:
                    // DEC r[y]
                    _pOpSrc8 = _decode_r[y];
                    DEC8();
                    break;
                case 6:
                    // LD r[y],n
                    _pOpSrc8 = &_imm8.FromPC();
                    _pOpDst8 = _decode_r[y];
                    LD8();
                    break;
                case 7: __debugbreak();
                default: __debugbreak();
                }
            }
            break;
        case 1:
            {
                __debugbreak();
            }
            break;
        case 2:
            // alu[y] r[z]
            _pOpSrc8 = _decode_r[z];
            (*this.*_decode_alu[y])();
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
                            // RET cc[y]
                            RET((*this.*_decode_cc[y])());
                            break;
                        case 4:
                            // LD ($FF00+n),A
                            _pOpSrc8 = &_regA;
                            _pOpDst8 = &_indImm.ForLDH();
                            LD8();
                            break;
                        case 5: __debugbreak();
                        case 6:
                            // LD A,($FF00+n)
                            _pOpSrc8 = &_indImm.ForLDH();
                            _pOpDst8 = &_regA;
                            LD8();
                            break;
                        case 7: __debugbreak();
                        default: __debugbreak();
                        }
                    }
                    break;
                case 1:
                    {
                        switch (q)
                        {
                        case 0: __debugbreak();
                            case 1:
                            {
                                switch (p)
                                {
                                case 0:
                                    RET(true);
                                    break;
                                case 1:
                                    // EXX
                                    // Not implemented in GB
                                    __debugbreak();
                                case 2:
                                    // JP HL
                                    _pOpSrc16 = &_regHL;
                                    JP(true);
                                    break;
                                case 3:
                                    // LD SP,HL
                                    _pOpSrc16 = &_regHL;
                                    _pOpDst16 = &_regSP;
                                    LD16();
                                    break;
                                default: __debugbreak();
                                }
                            }
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 2:
                    // JP cc[y],nn
                    _pOpSrc16 = &_imm16.FromPC();
                    JP((*this.*_decode_cc[y])());
                    break;
                case 3:
                    {
                        switch (y)
                        {
                        case 0:
                            // JP nn
                            _pOpSrc16 = &_imm16.FromPC();
                            JP(true);
                            break;
                        case 1:
                            // CB Prefix
                            __debugbreak();
                            break;
                        case 2:
                            // OUT(n),A
                            // Not implemented in GB
                            __debugbreak();
                            break;
                        case 3:
                            // IN A,(n)
                            // Not implemented in GB
                            __debugbreak();
                            break;
                        case 4:
                            // EX (SP),HL
                            // Not implemented in GB
                            __debugbreak();
                            break;
                        case 5:
                            // EX DE,HL
                            // Not implemented in GB
                            __debugbreak();
                            break;
                        case 6:
                            DI();
                            break;
                        case 7:
                            EI();
                            break;
                        default: __debugbreak();
                        }
                    }
                    break;
                case 4:
                    __debugbreak();
                    break;
                case 5:
                    __debugbreak();
                    break;
                case 6:
                    // alu[y] n
                    _pOpSrc8 = &_imm8.FromPC();
                    (*this.*_decode_alu[y])();
                    break;
                case 7:
                    __debugbreak();
                    break;
                default:
                    __debugbreak();
                    break;
                }
            }
            break;
        default:
            {
                __debugbreak();
            }
            break;
        }
    }
    else
    {
        switch (x)
        {
            case 0:
            {
                __debugbreak();
            }
            break;
        case 1:
            {
                __debugbreak();
            }
            break;
        case 2:
            {
                __debugbreak();
            }
            break;
        case 3:
            {
                __debugbreak();
            }
            break;
        default:
            {
                __debugbreak();
            }
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
/// Actions
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

void Cpu::ADD()
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

void Cpu::RET(bool cond)
{
    __debugbreak();
}

void Cpu::EI()
{
    _interrupt_ime = true;
}

void Cpu::DI()
{
    _interrupt_ime = false;
    _interrupt_ime_lag = false;
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