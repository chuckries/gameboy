#include "stdafx.h"
#include "input.h"
#include "gameboy.h"

Input::Input(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cpu(nullptr)
{
}

Input::~Input()
{
}

void Input::Init()
{
    _cpu = _gameboy._cpu;

    _dpad = 0x0F;
    _buttons = 0x0F;
}

void Input::UnInit()
{
    _cpu = nullptr;
}

u8 Input::Load()
{
    //return _p1 | 0b11001111;
    _p1 |= 0x0F;
    if ((_p1 & 0b00110000) == 0)
    {

    }
    else if ((_p1 & 0b00100000) == 0)
    {
        _p1 &= (_buttons | 0xF0);
    }
    else if ((_p1 & 0b00010000) == 0)
    {
        _p1 &= (_dpad | 0xF0);
    }

    _p1 |= 0x0b11000000; // these bits always set

    return _p1;
}

void Input::Store(u8 val)
{
    _p1 = val & 0b00110000;
}

void Input::Button(u8 idx, bool pressed)
{
    u8 mask = 0;
    switch (idx)
    {
    case 0:
    case 4:
        mask = 1 << 0;
        break;
    case 1:
    case 5:
        mask = 1 << 1;
        break;
    case 2:
    case 6:
        mask = 1 << 2;
        break;
    case 3:
    case 7:
        mask = 1 << 3;
        break;
    }

    u8& mode = idx < 4 ? _dpad : _buttons;

    if (pressed)
    {
        mode &= ~mask;
    }
    else
    {
        mode |= mask;
    }
}