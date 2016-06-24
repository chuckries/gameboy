#pragma once

class Gameboy;

class Cart
{
public:
    Cart(const Gameboy& gameboy);
    virtual ~Cart();

    void Init();
    void UnInit();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

    void LoadRom(const char* romPath);

private:
    const Gameboy& _gameboy;
    std::vector<u8> _rom;
};