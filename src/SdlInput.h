#pragma once

class Gameboy;

class SdlInput
{
public:
    SdlInput(Gameboy& gameboy);
    virtual ~SdlInput();

    void CheckInput();

private:
    void HandleKey(SDL_Keycode code, bool pressed);

private:
    Gameboy& _gameboy;
};