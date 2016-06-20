#include "stdafx.h"
#include "memory.h"
#include "cart.h"

MemoryMap::MemoryMap(std::shared_ptr<Cart> cart)
    : _cart(cart)
{

}

MemoryMap::~MemoryMap()
{

}

void MemoryMap::Init()
{

}

u8 MemoryMap::Load(u16 addr)
{
    if (addr < 0x8000)
    {
        return _cart->Load(addr);
    }
    else if (addr < 0xA000)
    {
        // VRAM
        __debugbreak();
    }
    else if (addr < 0xC000)
    {
        // Switchable RAM
        __debugbreak();
    }
    else if (addr < 0xFE00)
    {
        // RAM and Echo
        addr &= 0x1FFF;
        __debugbreak();
    }
    else if (addr < 0xFEA0)
    {
        // OAM
        __debugbreak();
    }
    else if (addr < 0xFF00)
    {
        // Unusable
        __debugbreak();
    }
    else if (addr < 0xFF4C)
    {
        // I/O
        __debugbreak();
    }
    else if (addr < 0xFFFF)
    {
        // Unusable
        __debugbreak();
    }
    else
    {
        // Interrupt Enable Register
        __debugbreak();
    }

    return 0;
}

void MemoryMap::Store(u16 addr, u8 val)
{
    __debugbreak();
}