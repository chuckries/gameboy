#include "stdafx.h"
#include "gameboy.h"
#include "cpu.h"

Gameboy::Gameboy()
{
    _cpu = std::make_unique<Cpu>();
}

Gameboy::~Gameboy()
{

}

void Gameboy::Init()
{
    _cpu->Init();
}

void Gameboy::LoadRom(const char* romPath)
{

}

void Gameboy::DoFrame()
{

}