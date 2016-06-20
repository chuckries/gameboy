#include "stdafx.h"
#include "gameboy.h"

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Error: Must pass path to ROM file.\n");
        return -1;
    }

    Gameboy gameboy;

    gameboy.Init();
    gameboy.LoadRom(argv[1]);

    for (;;)
    {
        gameboy.DoFrame();
    }
}