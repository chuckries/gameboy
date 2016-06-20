#pragma once

class Cpu;

class Gameboy
{
public:
    Gameboy();
    virtual ~Gameboy();

    void Init();
    void LoadRom(const char* romPath);

    void DoFrame();
private:
    std::unique_ptr<Cpu> _cpu;
};