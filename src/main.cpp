#include "stdafx.h"
#include "gameboy.h"
#include "cart.h"
#include "SdlGfx.h"
#include "SdlInput.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Error: Must pass path to ROM file.\n");
        return -1;
    }

    Gameboy gameboy;

    gameboy.Init(std::make_unique<StdRom>(argv[1]));

    SdlGfx gfx;
    SdlInput input(gameboy);
    u8 gbScreen[160 * 144] = { 0 };

    for (;;)
    {
        gameboy.DoFrame(gbScreen);
        gfx.Blit(gbScreen);
        input.CheckInput();
    }
}