#pragma once

class Gameboy;
class Cpu;

class Video
{
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

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    std::vector<u8> _vram;
    std::vector<u8> _oam;

    //constants
private:
    static const u32 CYCLES_PER_SCANLINE;
    static const u32 SCANLINES_PER_FRAME;
    static const u32 VBLANK_SCANLINE;

private:
    u32 _scanlineCycles;

    // Redering
private:
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
    u8 _obp0;
    u8 _obp1;
    u8 _ly;
};