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
    , _vram(0)
    , _oam(0)
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

    _vram.resize(0x2000, 0);
    _oam.resize(0xA0, 0);

    _scanlineCycles = 0;

    _lcdc = 0x91;
    _stat = 0;
    _ly = 0;
}

void Video::UnInit()
{
    _cpu = nullptr;
}

u8 Video::LoadVRam(u16 addr)
{
    addr &= 0x1FFF;
    return _vram[addr];
}

void Video::StoreVRam(u16 addr, u8 val)
{
    addr &= 0x1FFF;
    _vram[addr] = val;
}

u8 Video::LoadOAM(u16 addr)
{
    addr &= 0xFF;
    return _oam[addr];
}

void Video::StoreOAM(u16 addr, u8 val)
{
    addr &= 0xFF;
    _oam[addr] = val;
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