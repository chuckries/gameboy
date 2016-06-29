#include "stdafx.h"
#include "SdlInput.h"
#include "gameboy.h"

SdlInput::SdlInput(Gameboy& gameboy)
    : _gameboy(gameboy)
{
    SDL_InitSubSystem(SDL_INIT_EVENTS);
}

SdlInput::~SdlInput()
{
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void SdlInput::CheckInput()
{
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent))
    {
        switch (sdlEvent.type)
        {
        case SDL_KEYDOWN:
            if (sdlEvent.key.repeat == 0)
            {
                HandleKey(sdlEvent.key.keysym.sym, true);
            }
            break;
        case SDL_KEYUP:
            if (sdlEvent.key.repeat == 0)
            {
                HandleKey(sdlEvent.key.keysym.sym, false);
            }
            break;
        }
    }
}

void SdlInput::HandleKey(SDL_Keycode code, bool pressed)
{
    switch (code)
    {
    case SDLK_LALT:
    case SDLK_s:
        //_controller0->A(pressed);
        _gameboy.Button(4, pressed);
        break;
    case SDLK_LCTRL:
    case SDLK_a:
        //_controller0->B(pressed);
        _gameboy.Button(5, pressed);
        break;
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
    case SDLK_BACKSLASH:
        //_controller0->Select(pressed);
        _gameboy.Button(6, pressed);
        break;
    case SDLK_RETURN:
        //_controller0->Start(pressed);
        _gameboy.Button(7, pressed);
        break;
    case SDLK_UP:
        //_controller0->Up(pressed);
        _gameboy.Button(2, pressed);
        break;
    case SDLK_DOWN:
        //_controller0->Down(pressed);
        _gameboy.Button(3, pressed);
        break;
    case SDLK_LEFT:
        //_controller0->Left(pressed);
        _gameboy.Button(1, pressed);
        break;
    case SDLK_RIGHT:
        //_controller0->Right(pressed);
        _gameboy.Button(0, pressed);
        break;
    }
}