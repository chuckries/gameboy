#pragma once

class Cart;

class MemoryMap
{
    friend class Video;
public:
    MemoryMap(std::shared_ptr<Cart> cart);
    virtual ~MemoryMap();

    void Init();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

private:
    std::shared_ptr<Cart> _cart;

    // Work RAM C000 - DFFF
    // TODO: Half of this ram is swappable for CGB
    std::vector<u8> _wram;

    // High RAM FF80 - FFFE
    std::vector<u8> _hram;

    // Video RAM (Not 100% sure why it lives here, but Video has a reference to it)
    std::vector<u8> _vram;

    // I/O Registers
private:
    u8 _io_P1;      // FF00
    u8 _io_SB;      // FF01
    u8 _io_SC;      // FF02
    u8 _io_DIV;     // FF04
    u8 _io_TIMA;    // FF05
    u8 _io_TMA;     // FF06
    u8 _io_TAC;     // FF07
    u8 _io_IF;      // FF0F
    u8 _io_LCDC;    // FF40
    u8 _io_STAT;    // FF41
    u8 _io_SCY;     // FF42
    u8 _io_SCX;     // FF43
    u8 _io_LY;      // FF44
    u8 _io_LYC;     // FF45
    u8 _io_DMA;     // FF46
    u8 _io_BGP;     // FF47
    u8 _io_OBP0;    // FF48
    u8 _io_OBP1;    // FF49
    u8 _io_WY;      // FF4A
    u8 _io_WX;      // FF4B
    u8 _io_IE;      // FFFF
};