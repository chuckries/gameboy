#pragma once

class MemoryMap;

class Cpu
{
public:
    Cpu(std::shared_ptr<MemoryMap> mem);
    virtual ~Cpu();

    void Init();
    void Step();

private:
    void Decode();

    u8 LoadByteBumpPC();
    u16 LoadWordBumpPC();

private:
    std::shared_ptr<MemoryMap> _mem;

private:
    u8 _op;

    Pair _pc;
};