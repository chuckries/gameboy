#include "stdafx.h"
#include "memory.h"
#include "gameboy.h"
#include "cart.h"

MemoryMap::MemoryMap(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cart(nullptr)
{
}

MemoryMap::~MemoryMap()
{
}

void MemoryMap::Init()
{
    _cart = _gameboy._cart;

    _wram.resize(0x2000, 0);
    _hram.resize(0x80, 0);

    _io_TIMA = 0;
    _io_TMA = 0;
    _io_TAC = 0;
    _io_LCDC = 0x91;
    _io_SCY = 0;
    _io_SCX = 0;
    _io_LYC = 0;
    _io_BGP = 0xFC;
    _io_OBP0 = 0xFF;
    _io_OBP1 = 0xFF;
    _io_WX = 0;
    _io_WY = 0;
    _io_IE = 0;
}

void MemoryMap::UnInit()
{
    _cart = nullptr;
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
        switch (addr)
        {
        case 0xFF00:
            return _io_P1 | 0b00001111;
        case 0xFF01:
            __debugbreak();
            return _io_SB;
        case 0xFF02:
            __debugbreak();
            return _io_SC;
        case 0xFF04:
            return _io_DIV;
        case 0xFF05:
            return _io_TIMA;
        case 0xFF06:
            return _io_TMA;
        case 0xFF07:
            return _io_TAC;
        case 0xFF0F:
            return _io_IF;
        case 0xFF40:
            return _io_LCDC;
        case 0xFF41:
            return _io_STAT;
        case 0xFF42:
            return _io_SCY;
        case 0xFF43:
            return _io_SCX;
        case 0xFF44:
            return _io_LY;
        case 0xFF45:
            return _io_LYC;
        case 0xFF47:
            return _io_BGP;
        case 0xFF48:
            return _io_OBP0;
        case 0xFF49:
            return _io_OBP1;
        case 0xFF4A:
            return _io_WY;
        case 0xFF4B:
            return _io_WX;
        }
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
        return _io_IE;
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
        switch (addr)
        {
        case 0xFF00:
            _io_P1 = val & 0b00110000;
            break;
        case 0xFF01:
            _io_SB = val;
            break;
        case 0xFF02:
            _io_SC = val;
            if (val  & (1 << 7)) __debugbreak();
            break;
        case 0xFF04:
            _io_DIV = val;
            __debugbreak();
            break;
        case 0xFF05:
            __debugbreak();
            _io_TIMA = val;
            break;
        case 0xFF06:
            __debugbreak();
            _io_TMA = val;
            break;
        case 0xFF07:
            __debugbreak();
            _io_TAC = val;
            break;
        case 0xFF0F:
            _io_IF = val;
            break;
        case 0xFF40:
            _io_LCDC = val;
            break;
        case 0xFF41:
            _io_STAT = val;
            break;
        case 0xFF42:
            _io_SCY = val;
            break;
        case 0xFF43:
            _io_SCX = val;
            break;
        case 0xFF45:
            _io_LYC = val;
            break;
        case 0xFF46:
            _io_DMA = val;
            __debugbreak();
            break;
        case 0xFF47:
            _io_BGP = val;
            break;
        case 0xFF48:
            _io_OBP0 = val;
            break;
        case 0xFF49:
            _io_OBP1 = val;
            break;
        case 0xFF4A:
            _io_WY = val;
            break;
        case 0xFF4B:
            _io_WX = val;
            break;
        default: __debugbreak();
        }
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
        _io_IE = val;
    }
}