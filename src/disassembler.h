#pragma once

class MemoryMap;

class Disassembler
{
public:
    class Instruction
    {
        friend class Disassembler;
    public:
        std::string GetFormattedCodeBytes();
        std::string GetDisassemblyString();

    private:
        void Reset();

    private:
        std::vector<u8> _codeBytes;
        std::stringstream _ss;
    };

public:
    Disassembler(std::shared_ptr<MemoryMap> mem);
    virtual ~Disassembler();

    void Disassemble(u16 pc, Instruction& instr);

private:
    u8 Read8(u16 addr);
    u16 Read16(u16 addr);

    u8 Read8BumpPC();
    u16 Read16BumpPC();

    std::string Display8BumpPC();
    std::string Display16BumpPC();

private:
    std::shared_ptr<MemoryMap> _mem;
    u16 _pc;

private:
    std::vector<std::string> _decode_alu;
    std::vector<std::string> _decode_r;
    std::vector<std::string> _decode_rp;
    std::vector<std::string> _decode_rp2;
};