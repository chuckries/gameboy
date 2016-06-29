#include "stdafx.h"
#include "cart.h"
#include "gameboy.h"
#include <fstream>

MBC_IDENTIFIER Rom::MBCID()
{
    switch (_rom[0x147])
    {
    case 0x00:
    case 0x08:
    case 0x09:
        return MBC_ROM_ONLY;
    case 0x01:
    case 0x02:
    case 0x03:
        return MBC_1;
    default:
        __debugbreak();
        return MBC_UNKNOWN;
    }
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

MBC1::MBC1(const Rom& rom)
    : _rom(rom)
    , _addrOffset(0x4000)
{
}

MBC1::~MBC1()
{
}

u8 MBC1::LoadRom(u16 addr)
{
    return _rom[_addrOffset + (addr & 0x3FFF)];
}

void MBC1::StoreRom(u16 addr, u8 val)
{
    (void)addr;
    (void)val;
    __debugbreak();
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
    _rom->LoadFromFile();

    _mbcId = _rom->MBCID();
    switch (_mbcId)
    {
    case MBC_1:
        _mbc1 = std::make_unique<MBC1>(*_rom.get());
    }
}

void Cart::UnInit()
{
}

u8 Cart::Load(u16 addr)
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

void Cart::Store(u16 addr, u8 val)
{
    switch (_mbcId)
    {
    case MBC_ROM_ONLY:
        break;
    case MBC_1:
        _mbc1->StoreRom(addr, val);
    }
}