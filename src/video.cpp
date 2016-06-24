#include "stdafx.h"
#include "video.h"
#include "gameboy.h"
#include "cpu.h"
#include "memory.h"

const u32 Video::CYCLES_PER_SCANLINE = 456;
const u32 Video::SCANLINES_PER_FRAME = 154;

Video::Video(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cpu(nullptr)
    , _scanlineCycles(0)
    , _ly(0)
{
}

Video::~Video()
{

}

void Video::Init()
{
    _cpu = _gameboy._cpu;

    _scanlineCycles = 0;
    _ly = 0;
}

void Video::UnInit()
{
    _cpu = nullptr;
}

void Video::Step(u32 cycles)
{
    _scanlineCycles += cycles;
    if (_scanlineCycles >= CYCLES_PER_SCANLINE)
    {
        _scanlineCycles -= CYCLES_PER_SCANLINE;
        _ly++;
        if (_ly == SCANLINES_PER_FRAME)
        {
            _ly = 0;
        }
    }
}

u8 Video::LY()
{
    return _ly;
}