#pragma once

class Cart
{
public:
    Cart();
    virtual ~Cart();

    void Init();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

    void LoadRom(const char* romPath);

private:
    std::vector<u8> _rom;
};