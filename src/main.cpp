#include "stdafx.h"
#include "gameboy.h"
#include "SdlGfx.h"

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

    SdlGfx gfx;
    u8 gbScreen[160 * 144];

    for (;;)
    {
        gameboy.DoFrame(gbScreen);
        gfx.Blit(gbScreen);
    }

    gameboy.UnInit();
}