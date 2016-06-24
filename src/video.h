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

    void Step(u32 cycles);

    // I/O
public:
    u8 LY();

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

private:
    static const u32 CYCLES_PER_SCANLINE;
    static const u32 SCANLINES_PER_FRAME;
    u32 _scanlineCycles;

    // I/O
private:
    u8 _ly;
};