#include "stdafx.h"
#include "gameboy.h"
#include "cpu.h"
#include "video.h"
#include "memory.h"
#include "cart.h"
#include "timer.h"

Gameboy::Gameboy()
{
    _cart = std::make_shared<Cart>(*this);
    _memoryMap = std::make_shared<MemoryMap>(*this);
    _cpu = std::make_shared<Cpu>(*this);
    _video = std::make_shared<Video>(*this);
    _timer = std::make_shared<Timer>(*this);
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
    _timer->Init();
}

void Gameboy::UnInit()
{
    _cart->UnInit();
    _memoryMap->UnInit();
    _cpu->UnInit();
    _video->UnInit();
    _timer->UnInit();
}

void Gameboy::LoadRom(const char* romPath)
{
    _cart->LoadRom(romPath);
}

void Gameboy::DoFrame()
{
    u32 cycles = 0;
    for (;;)
    {
        cycles = _cpu->Step();
        _video->Step(cycles);
        _timer->Step(cycles);
    }
}
