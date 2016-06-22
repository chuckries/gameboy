#pragma once

class MemoryMap;
class Disassembler;

class Cpu
{
private:
    class IOperand8;
    class IOperand16;
public:
    Cpu(std::shared_ptr<MemoryMap> mem);
    virtual ~Cpu();

    void Init();
    void Step();

private:
    u8 Read8(u16 addr);
    u16 Read16(u16 addr);

    void Write8(u16 addr, u8 val);
    void Write16(u16 addr, u16 val);

    u8 Read8BumpPC();
    u16 Read16BumpPC();

    void CleanState();
    void DoInterrupt();
    void Decode();
    void Trace();

private:
    std::shared_ptr<MemoryMap> _mem;
    std::unique_ptr<Disassembler> _disassembler;

private:
    // interrupt stuff

    // Interrup Master Enabl (and lag)
    // Any pending interrupts will not occur until after the instruction afer the EI instruction
    // _interrupt_ime_lag will be seet one instruction after _interrupt_ime is set
    bool _interrupt_ime;
    bool _interrupt_ime_lag;

    // Registers
    Pair _PC;
#ifdef _SP
#undef _SP // this is defined in some msft header
#endif
    Pair _SP;

    struct
    {
        union
        {
            u16 W;
            struct
            {
                u8 F;
                u8 A;
            } B;
        } AF;

        union
        {
            u16 W;
            struct
            {
                u8 C;
                u8 B;
            } B;
        } BC;

        union
        {
            u16 W;
            struct
            {
                u8 E;
                u8 D;
            } B;
        } DE;

        union
        {
            u16 W;
            struct
            {
                u8 L;
                u8 H;
            } B;
        } HL;

    } _regs;

private:
    // operand abstractions
    class IOperand8
    {
    public:
        virtual u8 Read() = 0;
        virtual void Write(u8 val) = 0;
    };

    class IOperand16
    {
    public:
        virtual u16 Read() = 0;
        virtual void Write(u16 val) = 0;
    };

    class Immediate8 : public IOperand8
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
            Immediate = _cpu.Read8BumpPC();
            return *this;
        }

        virtual u8 Read()
        {
            return Immediate;
        }

        virtual void Write(u8 val)
        {
            __debugbreak();
        }

    private:
        Cpu& _cpu;
    };

    class Immediate16 : public IOperand16
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
            Immedate = _cpu.Read16BumpPC();
            return *this;
        }

        virtual u16 Read()
        {
            return Immedate;
        }

        virtual void Write(u16 val)
        {
            __debugbreak();
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
            Immediate = _cpu.Read16BumpPC();
            return *this;
        }

        IndirectImmediate& ForLDH()
        {
            Immediate = 0xFF00 | (u16)_cpu.Read8BumpPC();
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

    IOperand8* _pOpSrc8;
    IOperand8* _pOpDst8;
    IOperand16* _pOpSrc16;
    IOperand16* _pOpDst16;

    Immediate8 _imm8;
    Immediate16 _imm16;

    Register8 _regA;
    Register8 _regB;
    Register8 _regC;
    Register8 _regD;
    Register8 _regE;
    Register8 _regF;
    Register8 _regH;
    Register8 _regL;

    Register16 _regSP;
    Register16 _regAF;
    Register16 _regBC;
    Register16 _regDE;
    Register16 _regHL;

    IndirectReg _indBC;
    IndirectReg _indDE;
    IndirectReg _indHL;

    IndirectImmediate _indImm;

    // Decode Items
private:
    typedef void(Cpu::*AluOp)();
    typedef bool(Cpu::*CondOp)();

    std::vector<IOperand8*> _decode_r;
    std::vector<IOperand16*> _decode_rp;
    std::vector<IOperand16*> _decode_rp2;
    std::vector<AluOp> _decode_alu;
    std::vector<CondOp> _decode_cc;

    // Flag Operations
private:
    static const u8 Z_FLAG;
    static const u8 N_FLAG;
    static const u8 H_FLAG;
    static const u8 C_FLAG;
    void ResetFlags();
    void SetFlag(u8 flag, bool set);
    void SetZ(u8 val);
    void SetN();
    void ResetN();
    void SetH(bool set);
    void SetC(bool set);
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
    void ADD();
    void ADC();
    void SUB();
    void SBC();
    void AND();
    void XOR();
    void OR();
    void CP();
    void INC8();
    void DEC8();
    void SWAP();
    void DAA();
    void CPL();
    void CCF();
    void SCF();
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

    // helpers
    u8 sub_help();
};