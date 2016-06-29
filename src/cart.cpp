#include "stdafx.h"
#include "cart.h"
#include "gameboy.h"
#include <fstream>

Cart::Cart(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _rom(0)
{
}

Cart::~Cart()
{

}

void Cart::Init()
{
    _rom.clear();
}

void Cart::UnInit()
{
}

u8 Cart::Load(u16 addr)
{
    return _rom[addr & 0x7FFF];
}

void Cart::Store(u16 addr, u8 val)
{
    (void)addr;
    (void)val;
    __debugbreak();
}

void Cart::LoadRom(const char* romPath)
{
    std::ifstream ifs(romPath, std::ifstream::binary);
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
}