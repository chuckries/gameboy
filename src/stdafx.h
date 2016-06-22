#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>

typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

typedef std::int8_t i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;


union Pair
{
    u16 W;
    struct
    {
        u8 L;
        u8 H;
    } B;

    Pair()
    {
        W = 0;
    }
};