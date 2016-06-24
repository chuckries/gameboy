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

    void Step(u32 cycles);

    // I/O
public:
    u8 LY();

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    std::vector<u8> _vram;
    std::vector<u8> _oam;

private:
    static const u32 CYCLES_PER_SCANLINE;
    static const u32 SCANLINES_PER_FRAME;
    u32 _scanlineCycles;

    // I/O
private:
    u8 _ly;
};