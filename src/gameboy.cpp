#include "stdafx.h"
#include "gameboy.h"
#include "cpu.h"
#include "video.h"
#include "memory.h"
#include "cart.h"
#include "timer.h"
#include "input.h"

Gameboy::Gameboy()
{
    _cart = std::make_shared<Cart>(*this);
    _memoryMap = std::make_shared<MemoryMap>(*this);
    _cpu = std::make_shared<Cpu>(*this);
    _video = std::make_shared<Video>(*this);
    _timer = std::make_shared<Timer>(*this);
    _input = std::make_shared<Input>(*this);
}

Gameboy::~Gameboy()
{
    _cart->UnInit();
    _memoryMap->UnInit();
    _cpu->UnInit();
    _video->UnInit();
    _timer->UnInit();
    _input->UnInit();
}

void Gameboy::Init(std::unique_ptr<Rom> rom)
{
    _cart->Init(std::move(rom));
    _memoryMap->Init();
    _cpu->Init();
    _video->Init();
    _timer->Init();
    _input->Init();
}

static u8 scroll = 0;

void Gameboy::DoFrame(u8 gbScreen[])
{
    memset(gbScreen, 0, 160 * 144);
    scroll++;
    u32 cycles = 0;
    bool vblank = false;
    do
    {
        cycles = _cpu->Step();
        //_video->SCX = 0x11;
        //_video->SCY = scroll;
        vblank = _video->Step(cycles, gbScreen);
        _timer->Step(cycles);
    } while (!vblank);
}

void Gameboy::Button(u8 idx, bool pressed)
{
    _input->Button(idx, pressed);
}