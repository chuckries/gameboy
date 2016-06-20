#include "stdafx.h"
#include "gameboy.h"
#include "cpu.h"
#include "memory.h"
#include "cart.h"

Gameboy::Gameboy()
{
    _cart = std::make_shared<Cart>();
    _memoryMap = std::make_shared<MemoryMap>(_cart);

    _cpu = std::make_unique<Cpu>(_memoryMap);
}

Gameboy::~Gameboy()
{

}

void Gameboy::Init()
{
    _cart->Init();
    _memoryMap->Init();
    _cpu->Init();
}

void Gameboy::LoadRom(const char* romPath)
{
    _cart->LoadRom(romPath);
}

void Gameboy::DoFrame()
{
    for (;;)
    {
        _cpu->Step();
    }
}
