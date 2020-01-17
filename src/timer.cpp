#include "stdafx.h"
#include "timer.h"
#include "gameboy.h"
#include "cpu.h"

Timer::Timer(const Gameboy& gameboy)
    : _gameboy(gameboy)
    , _cpu(nullptr)
    , _cycles(0)
    , _div(0xABCC)
    , _intPending(false)
{
}

Timer::~Timer()
{

}

void Timer::Init()
{
    _cpu = _gameboy._cpu;

    WriteTIMA(0x00);
    WriteTMA(0x00);
    WriteTAC(0x00);
}

void Timer::UnInit()
{
    _cpu = nullptr;
}

void Timer::Step()
{
    int cycles = _cpu->GetCycles() - _cycles;
    _cycles = _cpu->GetCycles();

    for (u32 i = 0; i < cycles; i++)
    {
        if (_intPending)
        {
            _tima = _tma;
            _cpu->RequestInterrupt(Cpu::InterruptType::TIMER);
            _intPending = false;
        }
        else
        {
            u16 oldDiv = _div;
            _div++;

            // detect counter increment
            if (_timerEnabled)
            {
                bool oldDivBit = (oldDiv & (1 << _freqShift)) != 0;
                bool newDivBit = (_div & (1 << _freqShift)) != 0;
                if (oldDivBit && !newDivBit)
                {
                    _tima++;
                    if (_tima == 0)
                    {
                        _intPending = true;
                    }
                }
            }
        }
    }
}

u8 Timer::ReadDIV()
{
    Step();
    return (u8)((_div >> 8) & 0xFF);
}

void Timer::WriteDIV()
{
    Step();
    _div = 0;
}

u8 Timer::ReadTIMA()
{
    Step();
    return _tima;
}

void Timer::WriteTIMA(u8 val)
{
    Step();
    _tima = val;
}

u8 Timer::ReadTMA()
{
    Step();
    return _tma;
}

void Timer::WriteTMA(u8 val)
{
    Step();
    _tma = val;
}

u8 Timer::ReadTAC()
{
    Step();
    return _tac;
}

void Timer::WriteTAC(u8 val)
{
    Step();
    _tac = val;
    _timerEnabled = (_tac & (1 << 2)) != 0l;
    switch (_tac & 0x3)
    {
    case 0x00:
        _freqShift = 9;
        break;
    case 0x01:
        _freqShift = 3;
        break;
    case 0x02:
        _freqShift = 5;
        break;
    case 0x03:
        _freqShift = 7;
        break;
    default:
        break;
    }
}