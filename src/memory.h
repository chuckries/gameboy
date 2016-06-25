#pragma once

class Gameboy;
class Cart;
class Video;
class Timer;

class MemoryMap
{
public:
    MemoryMap(const Gameboy& gameboy);
    virtual ~MemoryMap();

    void Init();
    void UnInit();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cart> _cart;
    std::shared_ptr<Video> _video;
    std::shared_ptr<Timer> _timer;

    // Work RAM C000 - DFFF
    // TODO: Half of this ram is swappable for CGB
    std::vector<u8> _wram;

    // High RAM FF80 - FFFE
    std::vector<u8> _hram;

    // I/O Registers
private:
    u8 _io_P1;      // FF00
    u8 _io_SB;      // FF01
    u8 _io_SC;      // FF02
    u8 _io_BGP;     // FF47
    u8 _io_OBP0;    // FF48
    u8 _io_OBP1;    // FF49
};