#include "stdafx.h"
#include <SDL.h>
#include "SdlGfx.h"
#include <chrono>

SdlGfx::SdlGfx()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    _window = SDL_CreateWindow(
        "GameBoy",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        160 * 4,
        144 * 4,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
    );

    _renderer = SDL_CreateRenderer(
        _window,
        -1,
        SDL_RENDERER_ACCELERATED// | SDL_RENDERER_PRESENTVSYNC
    );

    _texture = SDL_CreateTexture(
        _renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );
}

SdlGfx::~SdlGfx()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SdlGfx::Blit(u8 gbScreen[])
{
    u8 palette[4] = { 0xFF, 0xD3, 0xA9, 0x00 };
    u8 screen[160 * 144 * 4];

    for (int i = 0; i < 160 * 144; i++)
    {
        screen[(i * 4) + 0] = palette[gbScreen[i]];
        screen[(i * 4) + 1] = palette[gbScreen[i]];
        screen[(i * 4) + 2] = palette[gbScreen[i]];
        screen[(i * 4) + 3] = 0;
    }

    SDL_UpdateTexture(_texture, NULL, (void*)&screen, 160 * 4);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    SDL_RenderPresent(_renderer);

    std::chrono::time_point<std::chrono::steady_clock> now;
    long long duration;
    do
    {
        now = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _lastDrawTime).count();
    } while (duration < 16666667); // 60 fps
    _lastDrawTime = now;
}