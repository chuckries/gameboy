#include "stdafx.h"
#include "cpu.h"
#include "memory.h"

Cpu::Cpu(std::shared_ptr<MemoryMap> mem)
    : _mem(mem)
{

}

Cpu::~Cpu()
{

}

void Cpu::Init()
{
    _op = 0;

    _pc.W = 0x100;
}

void Cpu::Step()
{
    _op = LoadByteBumpPC();

    Decode();
}

void Cpu::Decode()
{
    switch (_op)
    {
        // NOP
    case 0x00: break;

        // JP
    case 0xc3:
        _pc.W = LoadWordBumpPC();
        break;
    default:
        printf("Unimplemented Opcode: 0x%02X\n", _op);
        __debugbreak();
    }
}

u8 Cpu::LoadByteBumpPC()
{
    return _mem->Load(_pc.W++);
}

u16 Cpu::LoadWordBumpPC()
{
    Pair val;
    val.W = 0;
    val.B.L = LoadByteBumpPC();
    val.B.H = LoadByteBumpPC();
    return val.W;
}