#pragma once

class Gameboy;

class Video
{
public:
    Video(const Gameboy& gameboy);
    virtual ~Video();

    void Init();
    void UnInit();

    void Step();

private:
    const Gameboy& _gameboy;
};