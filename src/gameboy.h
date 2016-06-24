#pragma once

class Cpu;
class Video;
class MemoryMap;
class Cart;

class Gameboy
{
    friend class Cpu;
    friend class Video;
    friend class MemoryMap;
    friend class Cart;
public:
    Gameboy();
    virtual ~Gameboy();

    void Init();
    void UnInit();

    void LoadRom(const char* romPath);

    void DoFrame();
private:
    std::shared_ptr<Cpu> _cpu;
    std::shared_ptr<Video> _video;
    std::shared_ptr<MemoryMap> _memoryMap;
    std::shared_ptr<Cart> _cart;
};