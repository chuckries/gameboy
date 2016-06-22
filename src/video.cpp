#include "stdafx.h"
#include "video.h"
#include "memory.h"

Video::Video(std::shared_ptr<MemoryMap> mem)
    : _mem(mem)
    , _vram(_mem->_vram)
{
}

Video::~Video()
{

}

void Video::Init()
{

}

void Video::Step()
{

}