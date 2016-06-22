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
    _wram.resize(0x2000, 0);
    _hram.resize(0x80, 0);
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
        // Cartridge RAM
        __debugbreak();
    }
    else if (addr < 0xFE00)
    {
        // RAM and Echo
        // TODO: Half of this section is swappable for CGB
        addr &= 0x1FFF;
        return _wram[addr];
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
    else if (addr < 0xFF80)
    {
        // Unusable
        __debugbreak();
    }
    else if (addr < 0xFFFF)
    {
        // High RAM
        addr &= 0x7F;
        return _hram[addr];
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
    if (addr < 0x8000)
    {
        _cart->Store(addr, val);
    }
    else if (addr < 0xA000)
    {
        // VRAM
        __debugbreak();
    }
    else if (addr < 0xC000)
    {
        // Cartridge RAM
        __debugbreak();
    }
    else if (addr < 0xFE00)
    {
        // RAM and Echo
        // TODO: Half of this section is swappable for CGB
        addr &= 0x1FFF;
        _wram[addr] = val;
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
    else if (addr < 0xFF80)
    {
        // Unusable
        __debugbreak();
    }
    else if (addr < 0xFFFF)
    {
        // High RAM
        addr &= 0x7F;
        _hram[addr] = val;
    }
    else
    {
        // Interrupt Enable Register
        __debugbreak();
    }
}