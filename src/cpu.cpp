#include "stdafx.h"
#include "cpu.h"
#include "gameboy.h"
#include "memory.h"
#include "disassembler.h"

const u8 Cpu::Z_FLAG = (1 << 7);
const u8 Cpu::N_FLAG = (1 << 6);
const u8 Cpu::H_FLAG = (1 << 5);
const u8 Cpu::C_FLAG = (1 << 4);

const u32 Cpu::CYCLES[256] = {
/*       x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF */
/* 0x */  4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
/* 1x */  4, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/* 2x */  8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/* 3x */  8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/* 4x */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 5x */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 6x */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 7x */  8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
/* 8x */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 9x */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* Ax */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* Bx */  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* Cx */  8, 12, 12, 12, 12, 16,  8, 32,  8,  8, 12, 00, 12, 12,  8, 32,
/* Dx */  8, 12, 12, 00, 12, 16,  8, 32,  8,  8, 12, 00, 12, 00, 00, 32,
/* Ex */ 12, 12,  8, 00, 00, 16,  8, 32, 16,  4, 16, 00, 00, 00,  8, 32,
/* Fx */ 12, 12,  8,  4, 00, 16,  8, 32, 12,  8, 16,  4, 00, 00,  8, 32
};

const u32 Cpu::CB_CYCLES[8] = {
/* x0  x1  x2  x3  x4  x5  x6  x7 */
/* x8  x9  xA  xB  xC  xD  xE  xF */
    8,  8,  8,  8,  8,  8, 16,  8
};

Cpu::Cpu(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _mem(nullptr)
    , _disassembler(nullptr)
    , _cycles(0)
    , _interrupt_ime(false)
    , _interrupt_ime_lag(false)
    , _interrupt_if(0)
    , _interrupt_ie(0)
    , _pOpSrc8(nullptr)
    , _pOpDst8(nullptr)
    , _pOpRW8(nullptr)
    , _pOpSrc16(nullptr)
    , _pOpDst16(nullptr)
    , _pOpRW16(nullptr)
    , _imm8(*this)
    , _imm16(*this)
    , _indImm(*this)
    , _regs{ 0 }
    , _regA(_regs.AF.A)
    , _regB(_regs.BC.B)
    , _regC(_regs.BC.C)
    , _regD(_regs.DE.D)
    , _regE(_regs.DE.E)
    , _regH(_regs.HL.H)
    , _regL(_regs.HL.L)
    , _regAF(*_regs.AF)
    , _regBC(*_regs.BC)
    , _regDE(*_regs.DE)
    , _regHL(*_regs.HL)
    , _regSP(*_SP)
    , _indBC(*this, *_regs.BC)
    , _indDE(*this, *_regs.DE)
    , _indHL(*this, *_regs.HL)
    , _decode_r{ &_regB, &_regC, &_regD, &_regE, &_regH, &_regL, &_indHL, &_regA }
    , _decode_rp{ &_regBC, &_regDE, &_regHL, &_regSP }
    , _decode_rp2{ &_regBC, &_regDE, &_regHL, &_regAF }
    , _decode_cc{ &Cpu::CondNZ, &Cpu::CondZ, &Cpu::CondNC, &Cpu::CondC }
    , _decode_alu{ &Cpu::ADD8, &Cpu::ADC, &Cpu::SUB, &Cpu::SBC, &Cpu::AND, &Cpu::XOR, &Cpu::OR, &Cpu::CP }
    , _decode_rot{ &Cpu::RLC, &Cpu::RRC, &Cpu::RL, &Cpu::RR, &Cpu::SLA, &Cpu::SRA, &Cpu::SWAP, &Cpu::SRL }
{
}

Cpu::~Cpu()
{
}

void Cpu::Init()
{
    _mem = _gameboy._memoryMap;
    _disassembler = std::make_unique<Disassembler>(_mem);

    _cycles = 0;

    _interrupt_ime = true;
    _interrupt_ime_lag = true;

    _interrupt_ie = 0;
    _interrupt_if = 0;

    _isHalted = false;

    _PC = 0x100;
    _regs.AF = 0x01B0;
    _regs.BC = 0x0013;
    _regs.DE = 0x00D8;
    _regs.HL = 0x014D;
    _SP = 0xFFFE;

    _pOpSrc8 = nullptr;
    _pOpDst8 = nullptr;
    _pOpRW8 = nullptr;
    _pOpSrc16 = nullptr;
    _pOpDst16 = nullptr;
    _pOpRW16 = nullptr;

#ifdef TRACE
    _traceLog = fopen("C:\\Users\\chuckr\\desktop\\gb.log", "w");
#endif
}

void Cpu::UnInit()
{
    _mem = nullptr;
    _disassembler = nullptr;
}

u8 Cpu::Read8(u16 addr)
{
    if (addr == 0xFF0F)
    {
        return _interrupt_if;
    }
    else if (addr == 0xFFFF)
    {
        return _interrupt_ie;
    }
    else
    {
        return _mem->Load(addr);
    }
}

u16 Cpu::Read16(u16 addr)
{
    Word word;
    word._0 = Read8(addr);
    word._1 = Read8(addr + 1);
    return *word;
}

u8 Cpu::Read8BumpPC()
{
    return Read8(_PC++);
}

u16 Cpu::Read16BumpPC()
{
    Word word;
    word._0 = Read8BumpPC();
    word._1 = Read8BumpPC();
    return *word;
}

void Cpu::Write8(u16 addr, u8 val)
{
    if (addr == 0xFF0F)
    {
        _interrupt_if = val;
    }
    else if (addr == 0xFF46)
    {
        DMA(val);
    }
    else if (addr == 0xFFFF)
    {
        _interrupt_ie = val;
    }
    else
    {
        _mem->Store(addr, val);
    }
}

void Cpu::Write16(u16 addr, u16 val)
{
    Word word = val;
    Write8(addr, word._0);
    Write8(addr + 1, word._1);
}

u32 Cpu::Step()
{
    _cycles = 0;

    if (!DoInterrupt())
    {
        if (!_isHalted)
        {
            Decode();
        }
        else
        {
            _cycles = 4;
        }
    }
    else
    {
        _isHalted = false;
    }

    return _cycles;
}

void Cpu::RequestInterrupt(Cpu::InterruptType interrupt)
{
    _interrupt_if |= (u8)interrupt;
    if (_isHalted)
        _isHalted = ((_interrupt_if & _interrupt_ie) == 0);
}

bool Cpu::DoInterrupt()
{
    if (_interrupt_ime_lag)
    {
        u8 interruptJoin = _interrupt_ie & _interrupt_if & 0x1F;
        if (interruptJoin != 0)
        {
            // find the first bit set in both _interrupt_ie and _interrupt_if
            u8 i = 0;
            while ((interruptJoin & (1 << i)) == 0)
            {
                i++;
            }

            push_help(*_PC);
            _PC._1 = 0;
            switch (i)
            {
            case 0:
                _PC._0 = 0x40;
                break;
            case 1:
                _PC._0 = 0x48;
                break;
            case 2:
                _PC._0 = 0x50;
                break;
            case 3:
                _PC._0 = 0x58;
                break;
            case 4:
                _PC._0 = 0x60;
                break;
            default:
                __debugbreak();
                break;
            }

            DI();
            _interrupt_if &= ~(1 << i);

            return true;
        }
    }
    else if (_interrupt_ime)
    {
        _interrupt_ime_lag = true;
    }

    return false;
}

void Cpu::DMA(u8 val)
{
    Word srcAddr;
    srcAddr._1 = val;

    Word dstAddr;
    dstAddr._1 = 0xFE;

    for (u8 i = 0; i < 0xA0; i++)
    {
        srcAddr._0 = i;
        dstAddr._0 = i;
        Write8(*dstAddr, Read8(*srcAddr));
    }
}

void Cpu::Decode()
{
    Trace();

    u8 op = Read8BumpPC();

    bool prefix = op == 0xCB;
    if (prefix)
    {
        op = Read8BumpPC();
    }

    u8 y = (op >> 3) & 0b111;
    u8 z = op & 0b111;
    u8 p = (op >> 4) & 0b11;

    if (!prefix)
    {
        _cycles += CYCLES[op];
        switch (op)
        {
        case 0x00:
            // NOP
            break;
        case 0x08:
            // LD (nn),SP
            Write16(Read16BumpPC(), *_SP);
            break;
        case 0x010:
            // STOP
            Read8BumpPC(); // 0x00 byte
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
            {
                // JR cc[y-4],d
                bool cond = (*this.*_decode_cc[y - 4])();
                if (cond) _cycles += 4;
                JR(cond);
            }
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
            ADDHL();
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
            _regs.HL++;
            break;
        case 0x32:
            /// LDD (HL),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indHL;
            LD8();
            _regs.HL--;
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
            _pOpDst8 = &_regA;
            LD8();
            _regs.HL++;
            break;
        case 0x3A:
            // LDD A,(HL)
            _pOpSrc8 = &_indHL;
            _pOpDst8 = &_regA;
            LD8();
            _regs.HL--;
            break;
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
            // INC rp[p]
            _pOpRW16 = _decode_rp[p];
            INC16();
            break;
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
            // DEC rp[p]
            _pOpRW16 = _decode_rp[p];
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
            _pOpRW8 = _decode_r[y];
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
            _pOpRW8 = _decode_r[y];
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
            RLCA();
            break;
        case 0xF:
            // RRCA
            RRCA();
            break;
        case 0x17:
            // RLA
            RLA();
            break;
        case 0x1F:
            // RRA
            RRA();
            break;
        case 0x27:
            // DAA
            DAA();
            break;
        case 0x2F:
            // CPL
            CPL();
            break;
        case 0x37:
            // SCF
            SetC(true);
            ResetN();
            ResetH();
            break;
        case 0x3F:
            // CCF
            SetC(!CondC());
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
            _isHalted = true;
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
            {
                bool cond = (*this.*_decode_cc[y])();
                if (cond) _cycles += 12;
                RET(cond);
            }
            break;
        case 0xE0:
            // LD ($FF00+n),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indImm.ForLDH();
            LD8();
            break;
        case 0xE8:
            // ADD SP,n
            _pOpSrc8 = &_imm8.FromPC();
            ADDSP();
            break;
        case 0xF0:
            // LD A,($FF00+n)
            _pOpSrc8 = &_indImm.ForLDH();
            _pOpDst8 = &_regA;
            LD8();
            break;
        case 0xF8:
            // LDHL
            _pOpSrc8 = &_imm8.FromPC();
            LDHL();
            break;
        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
            // POP rp2[p]
            _pOpDst16 = _decode_rp2[p];
            POP();
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
            _pOpSrc16 = &_regHL;
            JP(true);
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
            {
                _pOpSrc16 = &_imm16.FromPC();
                bool cond = (*this.*_decode_cc[y])();
                if (cond) _cycles += 4;
                JP(cond);
            }
            break;
        case 0xE2:
            // LD ($FF00+C),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indImm.FromRegC();
            LD8();
            break;
        case 0xEA:
            // LD (nn),A
            _pOpSrc8 = &_regA;
            _pOpDst8 = &_indImm.FromPC();
            LD8();
            break;
        case 0xF2:
            // LD A,($FF00+C)
            _pOpSrc8 = &_indImm.FromRegC();
            _pOpDst8 = &_regA;
            LD8();
            break;
        case 0xFA:
            // LD A,(nn)
            _pOpSrc8 = &_indImm.FromPC();
            _pOpDst8 = &_regA;
            LD8();
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
            {
                _pOpSrc16 = &_imm16.FromPC();
                bool cond = (*this.*_decode_cc[y])();
                if (cond) _cycles += 12;
                CALL(cond);
            }
            break;
        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
            // PUSH rp2[p]
            _pOpSrc16 = _decode_rp2[p];
            PUSH();
            break;
        case 0xCD:
            // CALL nn
            _pOpSrc16 = &_imm16.FromPC();
            CALL(true);
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
            RST(y << 3);
            break;
        default:
            __debugbreak();
            break;
        }
    }
    else
    {
        _cycles += CB_CYCLES[op & 0x7];
        switch (op & 0xC0)
        {
        case 0x00:
            // rot[y] r[z]
            _pOpRW8 = _decode_r[z];
            (*this.*_decode_rot[y])();
            break;
        case 0x40:
            // BIT y,r[z]
            _pOpSrc8 = _decode_r[z];
            BIT(y);
            break;
        case 0x80:
            // RES y,r[z]
            _pOpRW8 = _decode_r[z];
            RES(y);
            break;
        case 0xC0:
            // SET y,r[z]
            _pOpRW8 = _decode_r[z];
            SET(y);
            break;
        default:
            __debugbreak();
            break;
        }
    }

    _pOpSrc8 = nullptr;
    _pOpDst8 = nullptr;
    _pOpRW8 = nullptr;
    _pOpSrc16 = nullptr;
    _pOpDst16 = nullptr;
    _pOpRW16 = nullptr;
}

// Flag Operations
void Cpu::ResetFlags()
{
    _regs.AF.F = 0;
}

void Cpu::SetFlag(u8 flag, bool set)
{
    if (set)
    {
        _regs.AF.F |= flag;
    }
    else
    {
        _regs.AF.F &= ~flag;
    }
}

void Cpu::SetZ(u8 val)
{
    SetFlag(Z_FLAG, val == 0);
}

void Cpu::ResetZ()
{
    SetFlag(Z_FLAG, false);
}

void Cpu::SetN()
{
    SetFlag(N_FLAG, true);
}

void Cpu::ResetN()
{
    SetFlag(N_FLAG, false);
}

void Cpu::SetH()
{
    SetFlag(H_FLAG, true);
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

void Cpu::ResetC()
{
    SetFlag(C_FLAG, false);
}

bool Cpu::CondFlag(u8 flag)
{
    return (_regs.AF.F & flag) != 0;
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
    _regs.HL = addsp_help();
}

void Cpu::PUSH()
{
    push_help(_pOpSrc16->Read());
}

void Cpu::POP()
{
    _pOpDst16->Write(pop_help());
    _regs.AF.F &= 0xF0; // just in case we pop in to AF, the lower nibble of F is always 0
}

void Cpu::ADD8()
{
    Word word;
    u8 left = _regs.AF.A;
    u8 right = _pOpSrc8->Read();
    word = left + right;
    _regs.AF.A = word._0;

    left &= 0x0F;
    right &= 0x0F;
    u8 val = left + right;

    SetZ(word._0);
    SetC(word._1 != 0);
    ResetN();
    SetH((val & 0xF0) != 0);
}

void Cpu::ADDHL()
{
    _regs.HL = add16_help(*_regs.HL, _pOpSrc16->Read());
}

void Cpu::ADDSP()
{
    _SP = addsp_help();
}

void Cpu::ADC()
{
    Word word;
    u8 left = _regs.AF.A;
    u8 right = _pOpSrc8->Read();
    word = left + right;
    if (CondC()) word++;
    _regs.AF.A = word._0;

    left &= 0x0F;
    right &= 0x0F;
    u8 val = left + right;
    if (CondC()) val++;

    SetZ(word._0);
    SetC(word._1 != 0);
    ResetN();
    SetH((val & 0xF0) != 0);
}

void Cpu::SUB()
{
    _regs.AF.A = sub_help();
}

void Cpu::SBC()
{
    Word word;
    u8 left = _regs.AF.A;
    u8 right = _pOpSrc8->Read();
    word = left - right;
    if (CondC()) word--;
    _regs.AF.A = word._0;

    left &= 0x0F;
    right &= 0x0F;
    u8 val = left - right;
    if (CondC()) val--;

    SetZ(word._0);

    SetC(word._1 != 0);
    SetN();
    SetH((val & 0xF0) != 0);
}

void Cpu::AND()
{
    _regs.AF.A = _regs.AF.A & _pOpSrc8->Read();
    SetZ(_regs.AF.A);
    ResetN();
    SetH();
    ResetC();
}

void Cpu::XOR()
{
    _regs.AF.A ^= _pOpSrc8->Read();
    ResetFlags();
    SetZ(_regs.AF.A);
}

void Cpu::OR()
{
    _regs.AF.A |= _pOpSrc8->Read();
    ResetFlags();
    SetZ(_regs.AF.A);
}

void Cpu::CP()
{
    sub_help();
}

void Cpu::INC8()
{
    u8 val = _pOpRW8->Read();
    u8 newVal = val + 1;
    _pOpRW8->Write(newVal);
    SetZ(newVal);
    ResetN();
    ResetH();
    val &= 0x0F;
    val++;
    SetH((val & 0xF0) != 0);
}

void Cpu::INC16()
{
    _pOpRW16->Write(_pOpRW16->Read() + 1);
}

void Cpu::DEC8()
{
    u8 val = _pOpRW8->Read();
    u8 newVal = val - 1;
    _pOpRW8->Write(newVal);
    SetZ(newVal);
    SetN();
    ResetH();
    val = (val & 0x0F);
    val--;
    SetH((val & 0xF0) != 0);
}

void Cpu::DEC16()
{
    _pOpRW16->Write(_pOpRW16->Read() - 1);
}

void Cpu::SWAP()
{
    u8 oldVal = _pOpRW8->Read();
    u8 newVal = 0;
    newVal |= ((oldVal >> 4) & 0x0F);
    newVal |= ((oldVal << 4) & 0xF0);
    _pOpRW8->Write(newVal);
    ResetFlags();
    SetZ(newVal);
}

void Cpu::DAA()
{
    int a = _regs.AF.A;

    if (!CondFlag(N_FLAG))
    {
        if (CondFlag(H_FLAG) || ((a & 0x0F) >= 0x0A))
        {
            a += 0x06;
        }

        if (CondC() || (a >= 0xA0))
        {
            a += 0x60;
        }
    }
    else
    {
        if (CondFlag(H_FLAG))
        {
            a = (a - 0x06) & 0xFF;
        }

        if (CondC())
        {
            a -= 0x60;
        }
    }

    _regs.AF.A = a & 0xFF;

    ResetH();
    SetZ(_regs.AF.A);
    SetC(((a & 0xFF00) != 0) || CondC());
}

void Cpu::CPL()
{
    _regs.AF.A = ~_regs.AF.A;
    SetH();
    SetN();
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
    _regs.AF.A = rlc_help(_regs.AF.A);
    ResetZ();
}

void Cpu::RLA()
{
    _regs.AF.A = rl_help(_regs.AF.A);
    ResetZ();
}

void Cpu::RRCA()
{
    _regs.AF.A = rrc_help(_regs.AF.A);
    ResetZ();
}

void Cpu::RRA()
{
    _regs.AF.A = rr_help(_regs.AF.A);
    ResetZ();
}

void Cpu::RLC()
{
    u8 val = rlc_help(_pOpRW8->Read());
    _pOpRW8->Write(val);
}

void Cpu::RL()
{
    u8 val = rl_help(_pOpRW8->Read());
    _pOpRW8->Write(val);
}

void Cpu::RRC()
{
    u8 val = rrc_help(_pOpRW8->Read());
    _pOpRW8->Write(val);
}

void Cpu::RR()
{
    u8 val = rr_help(_pOpRW8->Read());
    _pOpRW8->Write(val);
}

void Cpu::SLA()
{
    u8 val = shift_left_help(_pOpRW8->Read(), false);
    _pOpRW8->Write(val);
}

void Cpu::SRA()
{
    u8 val = _pOpRW8->Read();
    val = shift_right_help(val, bit_help(val, 7));
    _pOpRW8->Write(val);
}

void Cpu::SRL()
{
    u8 val = shift_right_help(_pOpRW8->Read(), false);
    _pOpRW8->Write(val);
}

void Cpu::BIT(u8 val)
{
    ResetN();
    SetH();
    SetZ(bit_help(_pOpSrc8->Read(), val));
}

void Cpu::SET(u8 val)
{
    _pOpRW8->Write(_pOpRW8->Read() | (1 << val));
}

void Cpu::RES(u8 val)
{
    _pOpRW8->Write(_pOpRW8->Read() & ~(1 << val));
}

void Cpu::JP(bool condition)
{
    u16 newPC = _pOpSrc16->Read();
    if (condition)
    {
        _PC = newPC;
    }
}

void Cpu::JR(bool condition)
{
    i8 disp = (i8)Read8BumpPC();
    if (condition)
    {
        i16 oldPC = (i16)*_PC;
        i16 newPC = oldPC + (i16)(disp);
        _PC = (u16)newPC;
    }
}

void Cpu::CALL(bool condition)
{
    u16 addr = _pOpSrc16->Read();
    if (condition)
    {
        push_help(*_PC);
        _PC = addr;
    }
}

void Cpu::RST(u8 val)
{
    push_help(*_PC);
    _PC._0 = val;
    _PC._1 = 0;
}

void Cpu::RET(bool cond)
{
    if (cond)
    {
        _PC = pop_help();
    }
}

void Cpu::RETI()
{
    EI();
    RET(true);
}

void Cpu::push_help(u16 val)
{
    _SP -= 2;
    Write16(*_SP, val);
}

u16 Cpu::pop_help()
{
    u16 val = Read16(*_SP);
    _SP += 2;
    return val;
}

u16 Cpu::add16_help(u16 left, u16 right)
{
    DWord dword;
    dword = left + right;

    ResetN();
    SetC(dword.W._1 != 0);

    left &= 0x0FFF;
    right &= 0x0FFF;
    u16 val = left + right;
    SetH((val & 0xF000) != 0);

    return dword.W._0;
}

u16 Cpu::addsp_help()
{
    u8 operand = _pOpSrc8->Read();
    i16 signedOperand = (i16)(i8)operand;

    u8 left = _SP._0;
    u8 right = operand;

    u16 result = (u16)((i16)(*_SP) + signedOperand);

    Word w = left + right;
    SetC(w._1 != 0);

    left &= 0x0F;
    right &= 0x0F;
    u8 val = left + right;
    SetH((val & 0xF0) != 0);

    ResetN();
    ResetZ();

    return result;
}

u8 Cpu::sub_help()
{
    Word word;
    u8 left = _regs.AF.A;
    u8 right = _pOpSrc8->Read();

    word = left - right;
    SetZ(word._0);
    SetN();
    SetC(word._1 != 0);

    left &= 0x0F;
    right &= 0x0F;
    left -= right;
    SetH((left & 0xF0) != 0);

    return word._0;
}

u8 Cpu::rlc_help(u8 val)
{
    return shift_left_help(val, bit_help(val, 7));
}

u8 Cpu::rl_help(u8 val)
{
    return shift_left_help(val, CondC());
}

u8 Cpu::rrc_help(u8 val)
{
    return shift_right_help(val, bit_help(val, 0));
}

u8 Cpu::rr_help(u8 val)
{
    return shift_right_help(val, CondC());
}

// Does not set Z Flag, caller must
u8 Cpu::shift_left_help(u8 val, bool lsb)
{
    bool newCarry = bit_help(val, 7);
    val <<= 1;
    val |= (lsb ? 1 : 0);
    SetC(newCarry);
    ResetH();
    ResetN();
    SetZ(val);
    return val;
}

u8 Cpu::shift_right_help(u8 val, bool msb)
{
    bool newCarry = bit_help(val, 0);
    val >>= 1;
    val |= (msb ? (1 << 7) : 0);
    SetC(newCarry);
    ResetH();
    ResetN();
    SetZ(val);
    return val;
}

bool Cpu::bit_help(u8 val, u8 bitNum)
{
    return (val & (1 << bitNum)) != 0;
}

#if defined(TRACE)
void Cpu::Trace()
{
    Disassembler::Instruction instr;
    _disassembler->Disassemble(*_PC, instr);

    fprintf(
        _traceLog,
        "%04X %-14s %-18s A:%02X B:%02X C:%02X D:%02X E:%02X F:%02X H:%02X L:%02X AF:%04X BC:%04X DE:%04X HL:%04X SP:%04X\n",
        *_PC,
        instr.GetFormattedCodeBytes().c_str(),
        instr.GetDisassemblyString().c_str(),
        _regs.AF.A,
        _regs.BC.B,
        _regs.BC.C,
        _regs.DE.D,
        _regs.DE.E,
        _regs.AF.F,
        _regs.HL.H,
        _regs.HL.L,
        *_regs.AF,
        *_regs.BC,
        *_regs.DE,
        *_regs.HL,
        *_SP
        );
}
#else
void Cpu::Trace() {}
#endif