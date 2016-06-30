#include "stdafx.h"
#include "memory.h"
#include "gameboy.h"
#include "cart.h"
#include "video.h"
#include "timer.h"
#include "input.h"

MemoryMap::MemoryMap(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cart(nullptr)
    , _video(nullptr)
{
}

MemoryMap::~MemoryMap()
{
}

void MemoryMap::Init()
{
    _cart = _gameboy._cart;
    _video = _gameboy._video;
    _timer = _gameboy._timer;
    _input = _gameboy._input;

    _wram.resize(0x2000, 0);
    _hram.resize(0x80, 0);
}

void MemoryMap::UnInit()
{
    _cart = nullptr;
    _video = nullptr;
    _timer = nullptr;
    _input = nullptr;
}

u8 MemoryMap::Load(u16 addr)
{
    if (addr < 0x8000)
    {
        // Cartridge ROM
        return _cart->LoadRom(addr);
    }
    else if (addr < 0xA000)
    {
        // VRAM
        return _video->LoadVRam(addr);
    }
    else if (addr < 0xC000)
    {
        // Cartridge RAM
        return _cart->LoadRam(addr);
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
        _video->LoadOAM(addr);
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
            return _input->Load();
        case 0xFF01:
            __debugbreak();
            return _io_SB;
        case 0xFF02:
            __debugbreak();
            return _io_SC;
        case 0xFF04:
            return _timer->ReadDIV();
        case 0xFF05:
            return _timer->ReadTIMA();
        case 0xFF06:
            return _timer->ReadTMA();
        case 0xFF07:
            return _timer->ReadTAC();
        case 0xFF10:
        case 0xFF11:
        case 0xFF12:
        case 0xFF13:
        case 0xFF14:
        case 0xFF15:
        case 0xFF16:
        case 0xFF17:
        case 0xFF18:
        case 0xFF19:
        case 0xFF1A:
        case 0xFF1B:
        case 0xFF1C:
        case 0xFF1D:
        case 0xFF1E:
        case 0xFF1F:
        case 0xFF20:
        case 0xFF21:
        case 0xFF22:
        case 0xFF23:
        case 0xFF24:
        case 0xFF25:
        case 0xFF26:
        case 0xFF27:
        case 0xFF28:
        case 0xFF29:
        case 0xFF2A:
        case 0xFF2B:
        case 0xFF2C:
        case 0xFF2D:
        case 0xFF2E:
        case 0xFF2F:
        case 0xFF30:
        case 0xFF31:
        case 0xFF32:
        case 0xFF33:
        case 0xFF34:
        case 0xFF35:
        case 0xFF36:
        case 0xFF37:
        case 0xFF38:
        case 0xFF39:
        case 0xFF3A:
        case 0xFF3B:
        case 0xFF3C:
        case 0xFF3D:
        case 0xFF3E:
        case 0xFF3F:
            // Sound Registers
            __debugbreak();
            break;
        case 0xFF40:
            return _video->ReadLCDC();
        case 0xFF41:
            return _video->ReadSTAT();
        case 0xFF42:
            return _video->SCY;
        case 0xFF43:
            return _video->SCX;
        case 0xFF44:
            return _video->LY();
        case 0xFF45:
            return _video->LYC;
        case 0xFF47:
            return _video->ReadBGP();
        case 0xFF48:
            return _video->ReadOBP0();
        case 0xFF49:
            return _video->ReadOBP1();
        case 0xFF4A:
            return _video->WY;
        case 0xFF4B:
            return _video->WX;
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
        __debugbreak();
    }

    return 0;
}

void MemoryMap::Store(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {
        // Cartridge ROM
        _cart->StoreRom(addr, val);
    }
    else if (addr < 0xA000)
    {
        // VRAM
        _video->StoreVRam(addr, val);
    }
    else if (addr < 0xC000)
    {
        // Cartridge RAM
        _cart->StoreRam(addr, val);
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
        _video->StoreOAM(addr, val);
    }
    else if (addr < 0xFF00)
    {
        // Unusable
    }
    else if (addr < 0xFF4C)
    {
        // I/O
        switch (addr)
        {
        case 0xFF00:
            _input->Store(val);
            break;
        case 0xFF01:
            _io_SB = val;
            break;
        case 0xFF02:
            _io_SC = val;
            break;
        case 0xFF03:
            // Undocumented
            break;
        case 0xFF04:
            _timer->WriteDIV();
            break;
        case 0xFF05:
            _timer->WriteTIMA(val);
            break;
        case 0xFF06:
            _timer->WriteTMA(val);
            break;
        case 0xFF07:
            _timer->WriteTAC(val);
            break;
        case 0xFF08:
        case 0xFF09:
        case 0xFF0A:
        case 0xFF0B:
        case 0xFF0C:
        case 0xFF0D:
        case 0xFF0E:
            // Undocumented
            break;
        case 0xFF10:
        case 0xFF11:
        case 0xFF12:
        case 0xFF13:
        case 0xFF14:
        case 0xFF15:
        case 0xFF16:
        case 0xFF17:
        case 0xFF18:
        case 0xFF19:
        case 0xFF1A:
        case 0xFF1B:
        case 0xFF1C:
        case 0xFF1D:
        case 0xFF1E:
        case 0xFF1F:
        case 0xFF20:
        case 0xFF21:
        case 0xFF22:
        case 0xFF23:
        case 0xFF24:
        case 0xFF25:
        case 0xFF26:
        case 0xFF27:
        case 0xFF28:
        case 0xFF29:
        case 0xFF2A:
        case 0xFF2B:
        case 0xFF2C:
        case 0xFF2D:
        case 0xFF2E:
        case 0xFF2F:
        case 0xFF30:
        case 0xFF31:
        case 0xFF32:
        case 0xFF33:
        case 0xFF34:
        case 0xFF35:
        case 0xFF36:
        case 0xFF37:
        case 0xFF38:
        case 0xFF39:
        case 0xFF3A:
        case 0xFF3B:
        case 0xFF3C:
        case 0xFF3D:
        case 0xFF3E:
        case 0xFF3F:
            // Sound Registers
            break;
        case 0xFF40:
            _video->WriteLCDC(val);
            break;
        case 0xFF41:
            _video->WriteSTAT(val);
            break;
        case 0xFF42:
            _video->SCY = val;
            break;
        case 0xFF43:
            _video->SCX = val;
            break;
        case 0xFF44:
            // TODO: what happens when write to LY?
            break;
        case 0xFF45:
            _video->LYC = val;
            break;
        case 0xFF47:
            _video->WriteBGP(val);
            break;
        case 0xFF48:
            _video->WriteOBP0(val);
            break;
        case 0xFF49:
            _video->WriteOBP1(val);
            break;
        case 0xFF4A:
            _video->WY = val;
            break;
        case 0xFF4B:
            _video->WX = val;
            break;
        default: __debugbreak();
        }
    }
    else if (addr < 0xFF80)
    {
        // Unusable
    }
    else if (addr < 0xFFFF)
    {
        // High RAM
        addr &= 0x7F;
        _hram[addr] = val;
    }
    else
    {
        __debugbreak();
    }
}