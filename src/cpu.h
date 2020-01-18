#pragma once

class Gameboy;
class MemoryMap;
class Disassembler;

class Cpu
{
public:
    enum class InterruptType : u8
    {
        V_BLANK = (1 << 0),
        STAT = (1 << 1),
        TIMER = (1 << 2),
        SERIAL = (1 << 3),
        INPUT = (1 << 4)
    };

public:
    Cpu(const Gameboy& gameboy);
    virtual ~Cpu();

    void Init();
    void UnInit();

    void BeforeFrame() { _cycles = 0; }
    void Step();
    void RequestInterrupt(InterruptType interrupt);

    u32 GetCycles() { return _cycles; }

private:
    u8 Read8(u16 addr);
    u16 Read16(u16 addr);

    void Write8(u16 addr, u8 val);
    void Write16(u16 addr, u16 val);

    u8 ReadPC8();
    u16 ReadPC16();

    bool DoInterrupt();
    void DMA(u8 val);
    void Decode();
    void Trace();

private:
    const Gameboy& _gameboy;
    std::shared_ptr<MemoryMap> _mem;
    std::unique_ptr<Disassembler> _disassembler;

    u32 _cycles;
    static const u32 CYCLES[256];
    static const u32 CB_CYCLES[8];

    // interrupt stuff
private:
    // Interrup Master Enabl (and lag)
    // Any pending interrupts will not occur until after the instruction afer the EI instruction
    // _interrupt_ime_lag will be seet one instruction after _interrupt_ime is set
    bool _interrupt_ime;
    bool _interrupt_ime_lag;

    u8 _interrupt_if;
    u8 _interrupt_ie;

    bool _isHalted;

    // Registers
private:
#define MAKE_REG(hi, lo) \
struct hi ## lo \
{ \
    u8 lo; \
    u8 hi; \
    u16& operator *() \
    { \
        return (u16&)*this; \
    } \
    u16& operator =(u16 val) \
    { \
        **this = val; \
        return **this; \
    } \
    u16& operator ++() \
    { \
        (**this)++; \
        return **this; \
    } \
    u16 operator ++(int) \
    { \
        hi ## lo tmp(*this); \
        operator++(); \
        return *tmp; \
    } \
    u16& operator --() \
    { \
        (**this)--; \
        return **this; \
    } \
    u16 operator --(int) \
    { \
        hi ## lo tmp(*this); \
        operator--(); \
        return *tmp; \
    } \
} hi ## lo;

    struct
    {
        MAKE_REG(A, F)
        MAKE_REG(B, C)
        MAKE_REG(D, E)
        MAKE_REG(H, L)
    } _regs;
    Word _PC;
    Word _SP;

    // Operands
private:
    class ISrcOperand8
    {
    public:
        virtual u8 Read() = 0;
    };

    class IDstOperand8
    {
    public:
        virtual void Write(u8 val) = 0;
    };

    class IOperand8 : public ISrcOperand8, public IDstOperand8
    {
    };

    class ISrcOperand16
    {
    public:
        virtual u16 Read() = 0;
    };

    class IDstOperand16
    {
    public:
        virtual void Write(u16 val) = 0;
    };

    class IOperand16 : public ISrcOperand16, public IDstOperand16
    {
    };

    class Immediate8 : public ISrcOperand8
    {
    public:
        u8 Immediate;

        Immediate8(Cpu& cpu)
            : _cpu(cpu)
            , Immediate(0)
        {
        }

        Immediate8& FromPC()
        {
            Immediate = _cpu.ReadPC8();
            return *this;
        }

        virtual u8 Read()
        {
            return Immediate;
        }

    private:
        Cpu& _cpu;
    };

    class Immediate16 : public ISrcOperand16
    {
    public:
        u16 Immedate;

        Immediate16(Cpu& cpu)
            : _cpu(cpu)
            , Immedate(0)
        {
        }

        Immediate16& FromPC()
        {
            Immedate = _cpu.ReadPC16();
            return *this;
        }

        virtual u16 Read()
        {
            return Immedate;
        }

    private:
        Cpu& _cpu;
    };

    class IndirectImmediate : public IOperand8
    {
    public:
        u16 Immediate;

        IndirectImmediate(Cpu& cpu)
            : _cpu(cpu)
            , Immediate(0)
        {
        }

        IndirectImmediate& FromPC()
        {
            Immediate = _cpu.ReadPC16();
            return *this;
        }

        IndirectImmediate& FromRegC()
        {
            Immediate = 0xFF00 | (u16)_cpu._regs.BC.C;
            return *this;
        }

        IndirectImmediate& ForLDH()
        {
            Immediate = 0xFF00 | (u16)_cpu.ReadPC8();
            return *this;
        }

        virtual u8 Read()
        {
            return _cpu.Read8(Immediate);
        }

        virtual void Write(u8 val)
        {
            _cpu.Write8(Immediate, val);
        }

    private:
        Cpu& _cpu;
    };

    class Register8 : public IOperand8
    {
    public:
        Register8(u8& reg)
            : _reg(reg)
        {
        }

        virtual u8 Read()
        {
            return _reg;
        }

        virtual void Write(u8 val)
        {
            _reg = val;
        }
    private:
        u8& _reg;
    };

    class Register16 : public IOperand16
    {
    public:
        Register16(u16& reg)
            : _reg(reg)
        {
        }

        virtual u16 Read()
        {
            return _reg;
        }

        virtual void Write(u16 val)
        {
            _reg = val;
        }

    private:
        u16& _reg;
    };

    class IndirectReg : public IOperand8
    {
    public:
        IndirectReg(Cpu& cpu, u16& reg)
            : _cpu(cpu)
            , _reg(reg)
        {
        }

        virtual u8 Read()
        {
            return _cpu.Read8(_reg);
        }

        virtual void Write(u8 val)
        {
            _cpu.Write8(_reg, val);
        }

    private:
        Cpu& _cpu;
        u16& _reg;
    };

    ISrcOperand8* _pOpSrc8;
    IDstOperand8* _pOpDst8;
    IOperand8* _pOpRW8;
    ISrcOperand16* _pOpSrc16;
    IDstOperand16* _pOpDst16;
    IOperand16* _pOpRW16;

    Immediate8 _imm8;
    Immediate16 _imm16;

    IndirectImmediate _indImm;

    Register8 _regA;
    Register8 _regB;
    Register8 _regC;
    Register8 _regD;
    Register8 _regE;
    Register8 _regH;
    Register8 _regL;

    Register16 _regAF;
    Register16 _regBC;
    Register16 _regDE;
    Register16 _regHL;
    Register16 _regSP;

    IndirectReg _indBC;
    IndirectReg _indDE;
    IndirectReg _indHL;

    // Decode Tables
private:
    typedef void(Cpu::*AluOp)();
    typedef bool(Cpu::*CondOp)();
    typedef void(Cpu::*RotOp)();

    std::vector<IOperand8*> _decode_r;
    std::vector<IOperand16*> _decode_rp;
    std::vector<IOperand16*> _decode_rp2;
    std::vector<CondOp> _decode_cc;
    std::vector<AluOp> _decode_alu;
    std::vector<RotOp> _decode_rot;

    // Flag Operations
private:
    static const u8 Z_FLAG;
    static const u8 N_FLAG;
    static const u8 H_FLAG;
    static const u8 C_FLAG;
    void ResetFlags();
    void SetFlag(u8 flag, bool set);
    void SetZ(u8 val);
    void ResetZ();
    void SetN();
    void ResetN();
    void SetH();
    void SetH(bool set);
    void ResetH();
    void SetC(bool set);
    void ResetC();
    bool CondFlag(u8 flag);
    bool CondNZ();
    bool CondZ();
    bool CondNC();
    bool CondC();

    // Operations
private:
    void LD8();
    void LD16();
    void LDHL();
    void PUSH();
    void POP();
    void ADD8();
    void ADDHL();
    void ADDSP();
    void ADC();
    void SUB();
    void SBC();
    void AND();
    void XOR();
    void OR();
    void CP();
    void INC8();
    void INC16();
    void DEC8();
    void DEC16();
    void SWAP();
    void DAA();
    void CPL();
    void HALT();
    void STOP();
    void DI();
    void EI();
    void RLCA();
    void RLA();
    void RRCA();
    void RRA();
    void RLC();
    void RL();
    void RRC();
    void RR();
    void SLA();
    void SRA();
    void SRL();
    void BIT(u8 val);
    void SET(u8 val);
    void RES(u8 val);
    void JP(bool cond);
    void JR(bool cond);
    void CALL(bool cond);
    void RST(u8 val);
    void RET(bool cond);
    void RETI();

    // Operation Helpers
    void push_help(u16 val);
    u16 pop_help();

    u16 add16_help(u16 left, u16 right);
    u16 addsp_help();
    u8 sub_help();

    u8 rlc_help(u8 val);
    u8 rl_help(u8 val);
    u8 rrc_help(u8 val);
    u8 rr_help(u8 val);

    // These do not set the Z flag, caller must
    u8 shift_left_help(u8 val, bool lsb);
    u8 shift_right_help(u8 val, bool msb);

    bool bit_help(u8 val, u8 bitNum);

#ifdef TRACE
    FILE* _traceLog;
#endif
};