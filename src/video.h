#pragma once

class Gameboy;
class Cpu;

class Video
{
private:
    struct Sprite
    {
        u8 _y;
        u8 _x;
        u8 TileNumber;
        u8 _attributes;

        u8 Y()
        {
            return _y - 16;
        }

        u8 X()
        {
            return _x - 8;
        }

        bool AboveBG()
        {
            return ((_attributes & (1 << 7)) != 0);
        }

        bool FlipY()
        {
            return ((_attributes & (1 << 6)) == 1);
        }

        bool FlipX()
        {
            return ((_attributes & (1 << 5)) == 1);
        }

        u8 GBPalette()
        {
            return (_attributes >> 4) & 1;
        }
    };
    static_assert(sizeof(Sprite) == 4, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _y) == 0, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _x) == 1, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, TileNumber) == 2, "Bad Sprite Struct");
    static_assert(offsetof(Sprite, _attributes) == 3, "Bad Sprite Struct");

public:
    Video(const Gameboy& gameboy);
    virtual ~Video();

    void Init();
    void UnInit();

    u8 LoadVRam(u16 addr);
    void StoreVRam(u16 addr, u8 val);
    u8 LoadOAM(u16 addr);
    void StoreOAM(u16 addr, u8 val);

    bool Step(u32 cycles, u8 gbScreen[]);

    // I/O
public:
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

    // Rendering
private:
    void DoScanline(u8 gbScreen[]);
    u8 GetBackgroundPixel(u8 x, u8 y);
    u8 GetWindowPixel(u8 x, u8 y);
    bool GetSpritePixel(u8 x, u8 y, u8 numSpritesOnLine, u8& sprColor, bool& sprHasPriority);
    u8 ProcessLineSprites(); // returns number of sprites on line
    void DoBackground(u8 gbScreen[]);
    void DoWindow(u8 gbScreen[]);
    void DoSprites(u8 gbScreen[]);

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    std::vector<u8> _vram;
    std::vector<u8> _oam;
    Sprite* _sprites; // an array of sprites that lays on top of the _oam byte array
    std::vector<Sprite*> _lineSprites; // holds pointers to sprites on the current line in priority order

    //constants
private:
    static const u32 CYCLES_PER_SCANLINE;
    static const u32 SCANLINES_PER_FRAME;
    static const u32 VBLANK_SCANLINE;
    static const u16 VRAM_MASK;

private:
    u32 _scanlineCycles;

    // Redering
private:
    u16 _bgTileMapAddr;
    u16 _bgTileDataAddr;
    bool _tileIndexIsSigned;

    bool _screenEnabled;
    bool _backgroundEnabled;
    bool _spritesEnabled;
    bool _bigSprites;
    bool _windowEnabled;

    // I/O
private:
    u8 _lcdc;
    u8 _stat;
    u8 _bgp;
    u8 _obp[2];
    u8 _ly;
};