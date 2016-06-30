#include "stdafx.h"
#include "timer.h"
#include "gameboy.h"
#include "cpu.h"

Timer::Timer(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cpu(nullptr)
{
}

Timer::~Timer()
{

}

void Timer::Init()
{
    _cpu = _gameboy._cpu;

    WriteDIV();
    WriteTIMA(0x00);
    WriteTMA(0x00);
    WriteTAC(0x00);
}

void Timer::UnInit()
{
    _cpu = nullptr;
}

void Timer::Step(u32 cycles)
{
    for (u32 i = 0; i < cycles; i++)
    {
        _div++;
    }
}

u8 Timer::ReadDIV()
{
    //__debugbreak();
    return _div;
}

void Timer::WriteDIV()
{
    _div = 0;
}

u8 Timer::ReadTIMA()
{
    //__debugbreak();
    return _tima;
}

void Timer::WriteTIMA(u8 val)
{
    _tima = val;
}

u8 Timer::ReadTMA()
{
    //__debugbreak();
    return _tma;
}

void Timer::WriteTMA(u8 val)
{
    _tma = val;
}

u8 Timer::ReadTAC()
{
    //__debugbreak();
    return _tac;
}

void Timer::WriteTAC(u8 val)
{
    _tac = val;
    if ((_tac & (1 << 2)) != 0)
    {
        //__debugbreak();
    }
}