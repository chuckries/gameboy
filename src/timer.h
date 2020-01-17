#pragma once

class Gameboy;
class Cpu;

class Timer
{
public:
    Timer(const Gameboy& gameboy);
    virtual ~Timer();

    void Init();
    void UnInit();

    void Step();

    u8 ReadDIV();
    void WriteDIV();
    u8 ReadTIMA();
    void WriteTIMA(u8 val);
    u8 ReadTMA();
    void WriteTMA(u8 val);
    u8 ReadTAC();
    void WriteTAC(u8 val);

    void AfterFrame() { _cycles -= 70224; }

private:
    const Gameboy& _gameboy;
    std::shared_ptr<Cpu> _cpu;

    // I/O Registers
    u16 _div;
    u8 _tima;
    u8 _tma;
    u8 _tac;

    bool _timerEnabled;
    u8 _freqShift;

    int _cycles;
    
    bool _intPending;
};