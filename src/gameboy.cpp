#include "stdafx.h"
#include "gameboy.h"
#include "cpu.h"
#include "video.h"
#include "memory.h"
#include "cart.h"

Gameboy::Gameboy()
{
    _cart = std::make_shared<Cart>(*this);
    _memoryMap = std::make_shared<MemoryMap>(*this);
    _cpu = std::make_unique<Cpu>(*this);
    _video = std::make_unique<Video>(*this);
}

Gameboy::~Gameboy()
{

}

void Gameboy::Init()
{
    _cart->Init();
    _memoryMap->Init();
    _cpu->Init();
    _video->Init();
}

void Gameboy::UnInit()
{
    _cart->UnInit();
    _memoryMap->UnInit();
    _cpu->UnInit();
    _video->UnInit();
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
        _video->Step();
    }
}
