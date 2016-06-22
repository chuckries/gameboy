#pragma once

class MemoryMap;

class Video
{
public:
    Video(std::shared_ptr<MemoryMap> mem);
    virtual ~Video();

    void Init();
    void Step();

private:
    std::shared_ptr<MemoryMap> _mem;
    std::vector<u8>& _vram; // vram lives in MemoryMap
};