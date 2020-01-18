#include "stdafx.h"
#include "video.h"
#include "gameboy.h"
#include "cpu.h"
#include "memory.h"

const u32 Video::CYCLES_PER_SCANLINE = 456;
const u32 Video::SCANLINES_PER_FRAME = 154;
const u32 Video::VBLANK_SCANLINE = 144;
const u16 Video::VRAM_MASK = 0x1FFF;

u8 Video::Tile::GetPixelData(u8 x, u8 y)
{
    u8 tileLo = _bytes[y * 2];
    u8 tileHi = _bytes[(y * 2) + 1];

    return ((tileLo >> (7 - x)) & 0x01) | (((tileHi >> (7 - x)) & 0x1) << 1);
}

u8 Video::Sprite::Y()
{
    return _y - 16;
}

u8 Video::Sprite::X()
{
    return _x - 8;
}

bool Video::Sprite::AboveBG()
{
    return ((_attributes & (1 << 7)) == 0);
}

bool Video::Sprite::FlipY()
{
    return ((_attributes & (1 << 6)) != 0);
}

bool Video::Sprite::FlipX()
{
    return ((_attributes & (1 << 5)) != 0);
}

u8 Video::Sprite::GBPalette()
{
    return (_attributes >> 4) & 1;
}

Video::Oam::Oam()
    : _mem{ 0 }
{
}

void Video::Oam::Init()
{
    memset(_mem.bytes, 0, sizeof(_mem.bytes));
    NumSpritesOnLine = 0;
}

u8 Video::Oam::operator[](int i) const
{
    return _mem.bytes[i];
}

u8& Video::Oam::operator[](int i)
{
    return _mem.bytes[i];
}

void Video::Oam::ProcessSpritesForLine(u8 y)
{
    NumSpritesOnLine = 0;
    for (u8 i = 0; i < 40 && NumSpritesOnLine < 10; i++)
    {
        Sprite* spr = &_mem.sprites[i];
        if (y >= spr->Y() && y < spr->Y() + SpriteHeight)
        {
            SpritesOnLine[NumSpritesOnLine++] = spr;
        }
    }

    std::sort(SpritesOnLine, SpritesOnLine + NumSpritesOnLine, [](Sprite* lhs, Sprite* rhs) {
        return (lhs->X() < rhs->X()) && (lhs < rhs);
    });
}

Video::Video(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cpu(nullptr)
    , _vram(0)
    , _cycles(0)
    , _vblankThisStep(false)
{
}

Video::~Video()
{

}

void Video::Init()
{
    _cpu = _gameboy._cpu;

    _vram.clear();
    _vram.resize(0x2000, 0);
    _spriteTiles = (Tile*)&_vram[0x8000 & VRAM_MASK];

    _oam.Init();

    _scanlineCycles = 0;

    WriteLCDC(0x91);
    WriteSTAT(0);
    SCY = 0;
    SCX = 0;
    _ly = 0;
    LYC = 0;
    WX = 0;
    WY = 0;
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
    Step();
    addr &= VRAM_MASK;
    return _vram[addr];
}

void Video::StoreVRam(u16 addr, u8 val)
{
    Step();
    addr &= VRAM_MASK;
    _vram[addr] = val;
}

u8 Video::LoadOAM(u16 addr)
{
    Step();
    addr &= 0xFF;
    return _oam[addr];
}

void Video::StoreOAM(u16 addr, u8 val)
{
    Step();
    addr &= 0xFF;
    _oam[addr] = val;
}

u8 Video::ReadLCDC()
{
    Step();
    return _lcdc;
}

void Video::WriteLCDC(u8 val)
{
    Step();
    _lcdc = val;

    _screenEnabled = (val & (1 << 7)) != 0;
    if (!_screenEnabled) _ly = 0;

    _bgTileMap = (val & (1 << 3)) == 0 ? &_vram[0x9800 & VRAM_MASK] : &_vram[0x9c00 & VRAM_MASK];
    _windowTileMap = (val & (1 << 6)) == 0 ? &_vram[0x9800 & VRAM_MASK] : &_vram[0x9c00 & VRAM_MASK];

    if ((val & (1 << 4)) != 0)
    {
        _bgTiles = (Tile*)&_vram[0x8000 & VRAM_MASK];
        _tileIndexIsSigned = false;
    }
    else
    {
        _bgTiles = (Tile*)&_vram[0x9000 & VRAM_MASK];
        _tileIndexIsSigned = true;
    }

    _backgroundEnabled = (val & (1 << 0)) != 0;
    _spritesEnabled = (val & (1 << 1)) != 0;
    _windowEnabled = (val & (1 << 5)) != 0;

    _oam.SpriteHeight = (val & (1 << 2)) == 0 ? 8 : 16;
}

u8 Video::ReadSTAT()
{
    Step();
    _stat &= 0xF8;
    _stat |= (_statMode & 0x3);
    if (_ly == LYC) _stat |= (1 << 2);

    return _stat;
}

void Video::WriteSTAT(u8 val)
{
    Step();
    _stat = (val & 0b01111000) | 0x80;
}

u8 Video::ReadBGP()
{
    Step();
    return _bgp;
}

void Video::WriteBGP(u8 val)
{
    Step();
    _bgp = val;
}

u8 Video::ReadOBP0()
{
    Step();
    return _obp[0];
}

void Video::WriteOBP0(u8 val)
{
    Step();
    _obp[0] = val;
}

u8 Video::ReadOBP1()
{
    Step();
    return _obp[1];
}

void Video::WriteOBP1(u8 val)
{
    Step();
    _obp[1] = val;
}

u8 Video::LY()
{
    Step();
    return _ly;
}

bool Video::Step()
{
    int cycles = _cpu->GetCycles() - _cycles;
    _cycles = _cpu->GetCycles();

    if (cycles == 0)
    {
        return _vblankThisStep;
    }

    if (_screenEnabled)
    {
        _scanlineCycles += cycles;

        u8 oldStatMode = _statMode;

        if (_scanlineCycles >= CYCLES_PER_SCANLINE)
        {
            if (_ly < VBLANK_SCANLINE)
            {
                DoScanline();
            }
            _ly++;
            if (_ly == VBLANK_SCANLINE)
            {
                _cpu->RequestInterrupt(Cpu::InterruptType::V_BLANK);
                //if ((_stat & (1 << 5)) != 0) _cpu->RequestInterrupt(Cpu::InterruptType::STAT);
                _vblankThisStep = true;
                _statMode = 1;
            }
            else if (_ly == SCANLINES_PER_FRAME)
            {
                _ly = 0;
            }

            if (_ly == LYC && ((_stat & (1 << 6)) != 0))
            {
                if ((_stat & (1 << 6)) != 0) _cpu->RequestInterrupt(Cpu::InterruptType::STAT);
            }

            _scanlineCycles -= CYCLES_PER_SCANLINE;
        }

        if (_ly < VBLANK_SCANLINE)
        {
            if (_scanlineCycles < 80)
            {
                _statMode = 2;
            }
            else if (_scanlineCycles < 252)
            {
                _statMode = 3;
            }
            else
            {
                _statMode = 0;
            }
        }
        else
        {
            _statMode = 1;
        }

        if (oldStatMode == 2 && _statMode == 3)
            _xLatch = SCX;

        if (_statMode != oldStatMode)
        {
            DoStatModeInterrupt();
        }
    }

    return _vblankThisStep;
}

void Video::DoStatModeInterrupt()
{
    if (_statMode < 3)
    {
        if ((_stat & (1 << (_statMode + 3))) != 0)
        {
            _cpu->RequestInterrupt(Cpu::InterruptType::STAT);
        }
    }
}

void Video::DoScanline()
{
    _oam.ProcessSpritesForLine(_ly);

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
            sprIsOpqaue = GetSpritePixel(i, _ly, sprColor, sprHasPriority);
        }

        u8 color = (_bgp >> (bgPaletteIndex * 2)) & 0x03;
        if (sprIsOpqaue)
        {
            if (sprHasPriority || bgPaletteIndex == 0)
            {
                color = sprColor;
            }
        }

        _screen[screenOffset + i] = color;
    }
}

u8 Video::GetBackgroundPixel(u32 x, u32 y)
{
    x += (u32)SCX; // _xLatch;
    x %= 256;
    y += (u32)SCY;
    y %= 256;

    u8 unsignedTileNum = _bgTileMap[((y / 8) * 32) + (x / 8)];

    Tile* tile;
    if (_tileIndexIsSigned)
    {
        tile = &_bgTiles[(i8)unsignedTileNum];
    }
    else
    {
        tile = &_bgTiles[unsignedTileNum];
    }

    u8 tileX = x % 8;
    u8 tileY = y % 8;
    return tile->GetPixelData(tileX, tileY);
}

u8 Video::GetWindowPixel(u8 x, u8 y)
{
    (void)x;
    (void)y;
    return 0;
}

bool Video::GetSpritePixel(u8 x, u8 y, u8& sprColor, bool& sprHasPriority)
{
    for (u8 i = 0; i < _oam.NumSpritesOnLine; i++)
    {
        Sprite* spr = _oam.SpritesOnLine[i];

        if (!(x >= spr->X() && (x < spr->X() + 8)))
        {
            continue;
        }

        u8 tileNumber = spr->TileNumber;
        if (_oam.SpriteHeight == 16)
        {
            tileNumber &= 0xFE;
            bool upperTile = (y - spr->Y()) < 8;
            if ((!spr->FlipY() && !upperTile) || (spr->FlipY() && upperTile))
            {
                tileNumber++;
            }
        }

        Tile* tile = &_spriteTiles[tileNumber];

        u8 tileY = (y - spr->Y()) % 8;
        if (spr->FlipY()) tileY = 7 - tileY;
        u8 tileX = (x - spr->X()) % 8;
        if (spr->FlipX())
        {
            tileX = 7 - tileX;
        }
        u8 paletteIndex = tile->GetPixelData(tileX, tileY);

        if (paletteIndex == 0)
        {
            continue;
        }

        sprColor = (_obp[spr->GBPalette()] >> (paletteIndex * 2)) & 0x03;
        sprHasPriority = spr->AboveBG();

        return true;
    }

    return false;
}