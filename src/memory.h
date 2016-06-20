#pragma once

class Cart;

class MemoryMap
{
public:
    MemoryMap(std::shared_ptr<Cart> cart);
    virtual ~MemoryMap();

    void Init();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

private:
    std::shared_ptr<Cart> _cart;
};