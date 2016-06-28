#include "stdafx.h"
#include <SDL.h>
#include "SdlInput.h"

SdlInput::SdlInput()
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

    }
}