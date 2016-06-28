#include "stdafx.h"
#include "video.h"
#include "gameboy.h"
#include "cpu.h"
#include "memory.h"

const u32 Video::CYCLES_PER_SCANLINE = 456;
const u32 Video::SCANLINES_PER_FRAME = 154;
const u32 Video::VBLANK_SCANLINE = 144;

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

    WriteLCDC(0x91);
    _stat = 0;
    WriteBGP(0xFC);
    WriteOBP0(0xFF);
    WriteOBP1(0xFF);
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

bool Video::Step(u32 cycles, u8 gbScreen[])
{
    bool vblank = false;

    if (_screenEnabled)
    {
        _scanlineCycles += cycles;
        if (_scanlineCycles >= CYCLES_PER_SCANLINE)
        {
            if (_ly < VBLANK_SCANLINE)
            {
                DoScanline(gbScreen);
            }
            _ly++;
            if (_ly == VBLANK_SCANLINE)
            {
                _cpu->RequestInterrupt(Cpu::InterruptType::V_BLANK);
                vblank = true;
            }
            else if (_ly == SCANLINES_PER_FRAME)
            {
                _ly = 0;
            }
            _scanlineCycles -= CYCLES_PER_SCANLINE;
        }
    }

    return vblank;
}

u8 Video::ReadLCDC()
{
    return _lcdc;
}

void Video::WriteLCDC(u8 val)
{
    _lcdc = val;

    _screenEnabled = (val & (1 << 7)) != 0;
    if (!_screenEnabled) _ly = 0;
}

u8 Video::ReadSTAT()
{
    return _stat;
}

void Video::WriteSTAT(u8 val)
{
    _stat = val;

    if ((_stat & 0x71) != 0)
    {
        __debugbreak();
    }
}

u8 Video::ReadBGP()
{
    return _bgp;
}

void Video::WriteBGP(u8 val)
{
    _bgp = val;
}

u8 Video::ReadOBP0()
{
    return _obp0;
}

void Video::WriteOBP0(u8 val)
{
    _obp0 = val;
}

u8 Video::ReadOBP1()
{
    return _obp1;
}

void Video::WriteOBP1(u8 val)
{
    _obp1 = val;
}

u8 Video::LY()
{
    return _ly;
}

void Video::DoScanline(u8 gbScreen[])
{
    u16 bgTileMapAddr = 0;
    u16 bgTileDataAddr = 0;
    bool indexIsSigned = false;

    // tile map
    if ((_lcdc & (1 << 3)) == 0)
    {
        bgTileMapAddr = 0x9800;
    }
    else
    {
        bgTileMapAddr = 0x9c00;
    }

    // tile data
    if ((_lcdc & (1 << 4)) == 0)
    {
        bgTileDataAddr = 0x8800;
        indexIsSigned = true;
    }
    else
    {
        bgTileDataAddr = 0x8000;
    }

    u8 y = _ly + SCY;

    bgTileMapAddr += ((y / 8) * 32);
    for (u8 x = 0; x < 20; x++)
    {
        i16 tileNum = 0;
        if (indexIsSigned)
        {
            tileNum = (i16)(i8)_vram[(bgTileMapAddr + x) & 0x1fff];
        }
        else
        {
            tileNum = (i16)(u16)_vram[(bgTileMapAddr + x) & 0x1fff];
        }

        u16 tileAddr = bgTileDataAddr + (tileNum * 16);
        tileAddr += (y % 8) * 2;

        u8 tileLo = _vram[tileAddr & 0x1fff];
        u8 tileHi = _vram[(tileAddr & 0x1fff) + 1];

        for (u8 tileX = 0; tileX < 8; tileX++)
        {
            u8 paletteIndex = ((tileLo >> (7 - tileX)) & 0x01) | (((tileHi >> (7 - tileX)) & 0x01) << 1);
            u8 color = (_bgp >> (paletteIndex * 2)) & 0x03;
            u32 screenIndex = (_ly * 160) + (x * 8) + tileX;
            gbScreen[screenIndex] = color;
        }
    }
}