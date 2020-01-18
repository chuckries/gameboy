#pragma once

#include "cpu.h"

class Gameboy;

class Video
{
    // Constants
private:
    static const u32 CYCLES_PER_SCANLINE;
    static const u32 SCANLINES_PER_FRAME;
    static const u32 VBLANK_SCANLINE;
    static const u16 VRAM_MASK;

    // Helper Structs/Classes
private:
    class Tile
    {
    private:
        Tile() { };

    public:
        u8 GetPixelData(u8 x, u8 y);

    private:
        u8 _bytes[16];
    };
    static_assert(sizeof(Tile) == 16, "Bad Tile Struct");

    struct Sprite
    {
    private:
        Sprite() { }

    public:
        u8 _y;
        u8 _x;
        u8 TileNumber;
        u8 _attributes;

        u8 Y();
        u8 X();
        bool AboveBG();
        bool FlipY();
        bool FlipX();
        u8 GBPalette();
    };
    static_assert(sizeof(Sprite) == 4, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _y) == 0, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _x) == 1, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, TileNumber) == 2, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _attributes) == 3, "Bad Sprite Struct");

    class Oam
    {
    public:
        Oam();
        void Init();
        u8 operator[](int i) const;
        u8& operator[](int i);
        void ProcessSpritesForLine(u8 y);

    public:
        Sprite* SpritesOnLine[10];
        u8 NumSpritesOnLine;
        u8 SpriteHeight;

    private:
        union
        {
            u8 bytes[0xA0];
            Sprite sprites[40];
        } _mem;
    };

public:
    Video(const Gameboy& gameboy);
    virtual ~Video();

    void Init();
    void UnInit();

    // I/O
public:
    u8 LoadVRam(u16 addr);
    void StoreVRam(u16 addr, u8 val);
    u8 LoadOAM(u16 addr);
    void StoreOAM(u16 addr, u8 val);

    u8 ReadLCDC();
    void WriteLCDC(u8 val);
    u8 ReadSTAT();
    void WriteSTAT(u8 val);
    u8 SCY;
    u8 SCX;
    u8 ReadBGP();
    void WriteBGP(u8 val);
    u8 ReadOBP0();
    void WriteOBP0(u8 val);
    u8 ReadOBP1();
    void WriteOBP1(u8 val);
    u8 LY();
    u8 LYC;
    u8 WY;
    u8 WX;

    bool Step();

    void SetScreen(u8 screen[])
    {
        _screen = screen;
    }

    void BeforeFrame()
    {
        _vblankThisStep = false;
        _cycles -= _cpu->GetCycles();
    }

private:
    void DoStatModeInterrupt();

    // Rendering
private:
    void DoScanline();
    u8 GetBackgroundPixel(u32 x, u32 y);
    u8 GetWindowPixel(u8 x, u8 y);
    bool GetSpritePixel(u8 x, u8 y, u8& sprColor, bool& sprHasPriority);

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    std::vector<u8> _vram;
    Oam _oam;
    u32 _scanlineCycles;

    bool _vblankThisStep;

    // I/O
private:
    u8 _lcdc;
    u8 _stat;
    u8 _bgp;
    u8 _obp[2];
    u8 _ly;

    u8 _statMode;

    // Rendering
private:
    Tile* _bgTiles;
    Tile* _spriteTiles;
    u8* _bgTileMap;
    u8* _windowTileMap;

    bool _tileIndexIsSigned;

    bool _screenEnabled;
    bool _backgroundEnabled;
    bool _spritesEnabled;
    bool _windowEnabled;

    int _cycles;

    u8* _screen;

    u8 _xLatch;
};