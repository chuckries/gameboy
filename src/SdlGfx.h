#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

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
};