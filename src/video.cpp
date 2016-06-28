#include "stdafx.h"
#include "video.h"
#include "gameboy.h"
#include "cpu.h"
#include "memory.h"
#include <algorithm>

const u32 Video::CYCLES_PER_SCANLINE = 456;
const u32 Video::SCANLINES_PER_FRAME = 154;
const u32 Video::VBLANK_SCANLINE = 144;
const u16 Video::VRAM_MASK = 0x1FFF;

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
    _sprites = (Sprite*)&_oam[0];
    _lineSprites.resize(40, nullptr);

    _scanlineCycles = 0;

    WriteLCDC(0x91);
    _stat = 0;
    SCY = 0;
    SCX = 0;
    _ly = 0;
    LYC = 0;
    WriteBGP(0xFC);
    WriteOBP0(0xFF);
    WriteOBP1(0xFF);
}

void Video::UnInit()
{
    _cpu = nullptr;
}

u8 Video::LoadVRam(u16 addr)
{
    addr &= VRAM_MASK;
    return _vram[addr];
}

void Video::StoreVRam(u16 addr, u8 val)
{
    addr &= VRAM_MASK;
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

    _bgTileMapAddr = (val & (1 << 3)) == 0 ? 0x9800 : 0x9c00;
    _bgTileDataAddr = (val & (1 << 4)) == 0 ? 0x8800 : 0x8000;
    _tileIndexIsSigned = (val & (1 << 4)) == 0;
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
    return _obp[0];
}

void Video::WriteOBP0(u8 val)
{
    _obp[0] = val;
}

u8 Video::ReadOBP1()
{
    return _obp[1];
}

void Video::WriteOBP1(u8 val)
{
    _obp[1] = val;
}

u8 Video::LY()
{
    return _ly;
}

void Video::DoScanline(u8 gbScreen[])
{
    u8 numSpritesOnLine = ProcessLineSprites();

    u32 screenOffset = _ly * 160;
    for (u8 i = 0; i < 160; i++)
    {
        u8 bgPaletteIndex = 0;
        if (_windowEnabled && ((WX - 7) >= i) && (WY <= _ly))
        {
            bgPaletteIndex = GetWindowPixel(i, _ly);
        }
        else if (_backgroundEnabled)
        {
            bgPaletteIndex = GetBackgroundPixel(i, _ly);
        }

        bool sprIsOpqaue = false;
        u8 sprColor = 0;
        bool sprHasPriority = false;
        if (_spritesEnabled)
        {
            sprIsOpqaue = GetSpritePixel(i, _ly, numSpritesOnLine, sprColor, sprHasPriority);
        }

        u8 color = (_bgp >> (bgPaletteIndex * 2)) & 0x03;
        if (sprIsOpqaue)
        {
            if (sprHasPriority || bgPaletteIndex == 0)
            {
                color = sprColor;
            }
        }

        gbScreen[screenOffset + i] = color;
    }
}

void Video::DoBackground(u8 gbScreen[])
{
    u8 y = _ly + SCY;
    u16 bgTileMapAddr = _bgTileMapAddr + ((y / 8) * 32);

    u8 tileX = SCX % 8;
    u8 lineIndex = 0;
    for (u8 x = 0; x < 21 && lineIndex < 160; x++)
    {
        i16 tileNum = 0;
        u16 bgTileMapAddrOffset = bgTileMapAddr + ((x + (SCX / 8)) % 32);
        if (_tileIndexIsSigned)
        {
            tileNum = (i16)(i8)_vram[bgTileMapAddrOffset & VRAM_MASK];
        }
        else
        {
            tileNum = (i16)(u16)_vram[bgTileMapAddrOffset & VRAM_MASK];
        }

        u16 tileAddr = _bgTileDataAddr + (tileNum * 16) + ((y % 8) * 2);

        u8 tileLo = _vram[tileAddr & VRAM_MASK];
        u8 tileHi = _vram[(tileAddr & VRAM_MASK) + 1];

        for (; tileX < 8 && lineIndex < 160; tileX++, lineIndex++)
        {
            u8 paletteIndex = ((tileLo >> (7 - tileX)) & 0x01) | (((tileHi >> (7 - tileX)) & 0x01) << 1);
            u8 color = (_bgp >> (paletteIndex * 2)) & 0x03;
            u32 screenIndex = (_ly * 160) + lineIndex;
            gbScreen[screenIndex] = color;
        }
        tileX = 0;
    }
}

u8 Video::GetBackgroundPixel(u8 x, u8 y)
{
    y += SCY;
    x += SCX;
    u16 bgTileMapAddr = _bgTileMapAddr + ((y / 8) * 32) + (x / 8);
    i16 tileNum = 0;
    if (_tileIndexIsSigned)
    {
        tileNum = (i16)(i8)_vram[bgTileMapAddr & VRAM_MASK];
    }
    else
    {
        tileNum = (i16)(u16)_vram[bgTileMapAddr & VRAM_MASK];
    }

    u16 tileAddr = _bgTileDataAddr + (tileNum * 16) + ((y % 8) * 2);

    u8 tileLo = _vram[tileAddr & VRAM_MASK];
    u8 tileHi = _vram[(tileAddr & VRAM_MASK) + 1];

    u8 tileX = x % 8;
    return ((tileLo >> (7 - tileX)) & 0x01) | (((tileHi >> (7 - tileX)) & 0x01) << 1);
}

u8 Video::GetWindowPixel(u8 x, u8 y)
{
    return 0;
}

bool Video::GetSpritePixel(u8 x, u8 y, u8 numSpritesOnLine, u8& sprColor, bool& sprHasPriority)
{
    auto begin = _lineSprites.begin();
    auto end = begin + numSpritesOnLine;
    auto it = std::find_if(begin, end, [x](Sprite* spr)
    {
        return x >= spr->X()  && (x <spr->X() + 8);
    });

    if (it == end)
    {
        return 0;
    }

    Sprite* spr = *it;
    u16 tileAddr = 0x8000 + (spr->TileNumber * 16) + ((y % 8) * 2);

    u8 tileLo = _vram[tileAddr & VRAM_MASK];
    u8 tileHi = _vram[(tileAddr & VRAM_MASK) + 1];

    u8 tileX = x % 8;
    u8 paletteIndex = ((tileLo >> (7 - tileX)) & 0x01) | (((tileHi >> (7 - tileX)) & 0x01) << 1);
    if (paletteIndex == 0)
    {
        return false;
    }

    sprColor = (_obp[spr->GBPalette()] >> (paletteIndex * 2)) & 0x03;
    sprHasPriority = spr->AboveBG();

    return true;
}

u8 Video::ProcessLineSprites()
{
    u8 numSprites = 0;
    for (u8 i = 0; i < 40; i++)
    {
        Sprite* spr = &_sprites[i];
        if (_ly >= spr->Y() && _ly < spr->Y() + 8)
        {
            _lineSprites[numSprites++] = spr;
        }
    }

    std::sort(_lineSprites.begin(), _lineSprites.begin() + numSprites, [](Sprite* lhs, Sprite* rhs) {
        return (lhs->X() < rhs->X()) && (lhs < rhs);
    });

    return numSprites <= 10 ? numSprites : 10;
}