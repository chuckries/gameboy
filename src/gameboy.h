#pragma once

class Cpu;
class Video;
class MemoryMap;
class Cart;
class Timer;

class SdlGfx;

class Gameboy
{
    friend class Cpu;
    friend class Video;
    friend class MemoryMap;
    friend class Cart;
    friend class Timer;
public:
    Gameboy();
    virtual ~Gameboy();

    void Init();
    void UnInit();

    void LoadRom(const char* romPath);

    void DoFrame(u8 gbScreen[]);
private:
    std::shared_ptr<Cpu> _cpu;
    std::shared_ptr<Video> _video;
    std::shared_ptr<MemoryMap> _memoryMap;
    std::shared_ptr<Cart> _cart;
    std::shared_ptr<Timer> _timer;
};