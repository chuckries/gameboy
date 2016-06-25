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

    _div = 0;
    _tima = 0;
    _tma = 0;
    _tac = 0;
}

void Timer::UnInit()
{
    _cpu = nullptr;
}

void Timer::Step(u32 cycles)
{

}

u8 Timer::ReadDIV()
{
    __debugbreak();
    return _div;
}

void Timer::WriteDIV()
{
    __debugbreak();
    _div = 0;
}

u8 Timer::ReadTIMA()
{
    __debugbreak();
    return _tima;
}

void Timer::WriteTIMA(u8 val)
{
    __debugbreak();
    _tima = val;
}

u8 Timer::ReadTMA()
{
    __debugbreak();
    return _tma;
}

void Timer::WriteTMA(u8 val)
{
    __debugbreak();
    _tma = val;
}

u8 Timer::ReadTAC()
{
    __debugbreak();
    return _tac;
}

void Timer::WriteTAC(u8 val)
{
    __debugbreak();
    _tac = val;
}