#include "stdafx.h"
#include "cart.h"
#include "gameboy.h"
#include <fstream>

bool Rom::Init()
{
    if (LoadFromFile())
    {
        switch (_rom[0x147])
        {
        case 0x09: HasSave = true;
        case 0x08: HasRam = true;
        case 0x00: MBCID = MBC_ROM_ONLY; break;

        case 0x03: HasSave = true;
        case 0x02: HasRam = true;
        case 0x01: MBCID = MBC_1; break;

        default:
            __debugbreak();
            MBCID = MBC_UNKNOWN;
            break;
        }

        if (HasRam)
        {
            switch (_rom[0x149])
            {
            case 0x00: RamSize = 0x0000; HasRam = false;  break;
            case 0x01: RamSize = 0x0800; break;
            case 0x02: RamSize = 0x2000; break;
            case 0x03: RamSize = 0x8000; break;
            case 0x04: RamSize = 0x20000; break;
            default:
                __debugbreak();
                RamSize = 0;
                break;
            }
        }

        return true;
    }

    return false;
}

StdRom::StdRom(const char* filePath)
    : _filePath(filePath)
{
}

bool StdRom::LoadFromFile()
{
    std::ifstream ifs(_filePath, std::ifstream::binary);
    if (ifs)
    {
        // get size of ROM file
        ifs.seekg(0, ifs.end);
        long long size = ifs.tellg();
        _rom.resize(size);
        ifs.seekg(0, ifs.beg);

        ifs.read((char*)&_rom[0], size);
        ifs.close();
    }

    return true;
}

MbcBase::MbcBase(const Rom& rom)
    : _rom(rom)
    , _romOffset(0x4000)
    , _ramOffset(0x0000)
    , _ram(rom.RamSize, 0xFF)
{
}

MbcBase::~MbcBase()
{
}

u8 MbcBase::LoadRom(u16 addr)
{
    return _rom[_romOffset + (addr & 0x3FFF)];
}

void MbcBase::StoreRom(u16 addr, u8 val)
{
    // nothing for Rom Only
}

u8 MbcBase::LoadRam(u16 addr)
{
    if (!_rom.HasRam)
    {
        __debugbreak();
        return 0xFF;
    }
    else
        return _ram[_ramOffset + (addr - 0xA000)];
}

void MbcBase::StoreRam(u16 addr, u8 val)
{
    if (!_rom.HasRam)
    { }
        //__debugbreak();
    else
        _ram[_ramOffset + (addr - 0xA000)] = val;
}

Mbc1::Mbc1(const Rom& rom)
    : MbcBase(rom)
{
}

Mbc1::~Mbc1()
{
}

void Mbc1::StoreRom(u16 addr, u8 val)
{
    if (addr < 0x2000)
    {
        _ramEnabled = (val & 0x0A) == 0x0A;
    }
    else if (addr < 0x4000)
    {
        _reg2000 = val & 0x1F;
        CalculateOffsets();
    }
    else if (addr < 0x6000)
    {
        _reg4000 = val & 0x03;
        CalculateOffsets();
    }
    else if (addr < 0x8000)
    {
        _reg6000 = ((val & 1) != 0);
        CalculateOffsets();
    }
    else
    {
        __debugbreak();
    }
}

void Mbc1::CalculateOffsets()
{
    u32 romBank = _reg2000;
    u32 ramBank = 0;

    if (romBank == 0)
    {
        romBank++;
    }

    if (!_reg6000)
    {
        // ROM Banking Mode
        romBank |= (_reg6000 << 5);
    }
    else
    {
        ramBank = _reg6000;
    }

    _romOffset = romBank * 0x4000;
    _ramOffset = ramBank & 0x2000;
}

Cart::Cart(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _rom(nullptr)
    , _mbcId(MBC_ROM_ONLY)
    , _mbc1(nullptr)
{
}

Cart::~Cart()
{
}

void Cart::Init(std::unique_ptr<Rom> rom)
{
    _rom = std::move(rom);
    _rom->Init();

    _mbcId = _rom->MBCID;
    switch (_mbcId)
    {
    case MBC_1:
        _mbc1 = std::make_unique<Mbc1>(*_rom.get());
    }
}

void Cart::UnInit()
{
}

u8 Cart::LoadRom(u16 addr)
{
    switch (addr & 0x4000)
    {
    case 0x0000:
        return (*_rom)[addr & 0x3FFF];
    case 0x4000:
        switch (_mbcId)
        {
        case MBC_ROM_ONLY:
            return (*_rom)[addr & 0x7FFF];
        case MBC_1:
            return _mbc1->LoadRom(addr & 0x3FFF);
        }
        break;
    default:
        __debugbreak();
        break;
    }

    return 0;
}

void Cart::StoreRom(u16 addr, u8 val)
{
    switch (_mbcId)
    {
    case MBC_ROM_ONLY:
        break;
    case MBC_1:
        _mbc1->StoreRom(addr, val);
        break;
    }
}

u8 Cart::LoadRam(u16 addr)
{
    switch (_mbcId)
    {
    case MBC_ROM_ONLY:
        __debugbreak();
        return 0xFF;
        break;
    case MBC_1:
        return _mbc1->LoadRam(addr);
        break;
    }
}

void Cart::StoreRam(u16 addr, u8 val)
{
    switch (_mbcId)
    {
    case MBC_ROM_ONLY:
        break;
    case MBC_1:
        _mbc1->StoreRam(addr, val);
        break;
    }
}