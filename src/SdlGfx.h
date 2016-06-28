#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

#include <chrono>

class SdlGfx
{
public:
    SdlGfx();
    virtual ~SdlGfx();

    void Blit(u8 gbScreen[]);

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;

    std::chrono::time_point<std::chrono::steady_clock> _lastDrawTime;
};