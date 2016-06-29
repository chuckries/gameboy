#pragma once

class Gameboy;
class Cpu;

class Input
{
public:
    Input(const Gameboy& gameboy);
    virtual ~Input();

    void Init();
    void UnInit();

    u8 Load();
    void Store(u8 val);

    void Button(u8 idx, bool pressed);

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    u8 _p1;
    u8 _buttons;
    u8 _dpad;
};