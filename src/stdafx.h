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

struct Word
{
    u8 _0;
    u8 _1;

    u16& operator *()
    {
        return (u16&)*this;
    }

    u16& operator =(u16 val)
    {
        **this = val;
        return **this;
    }

    u16& operator ++()
    {
        (**this)++;
        return **this;
    }

    u16 operator ++(int)
    {
        Word tmp(*this);
        operator++();
        return *tmp;
    }

    u16& operator --()
    {
        (**this)--;
        return **this;
    }

    u16 operator --(int)
    {
        Word tmp(*this);
        operator--();
        return *tmp;
    }

    u16& operator +=(u16 val)
    {
        (**this) += val;
        return **this;
    }

    u16& operator -=(u16 val)
    {
        (**this) -= val;
        return **this;
    }

    Word()
    {
        **this = 0;
    }

    Word(u16 val)
    {
        **this = val;
    }
};
static_assert(sizeof(Word) == sizeof(u16), "Bad Word Struct");
static_assert(offsetof(Word, _0) == 0, "Bad DWord Struct");
static_assert(offsetof(Word, _1) == 1, "Bad DWord Struct");

union DWord
{
    struct W
    {
        u16 _0;
        u16 _1;
    } W;

    struct B
    {
        u8 _0;
        u8 _1;
        u8 _2;
        u8 _3;
    } B;

    u32& operator *()
    {
        return (u32&)*this;
    }

    u32& operator =(u32 val)
    {
        **this = val;
        return **this;
    }

    u32& operator ++()
    {
        (**this)++;
        return **this;
    }

    u32& operator --()
    {
        (**this)--;
        return **this;
    }

    u32& operator +=(u32 val)
    {
        (**this) += val;
        return **this;
    }

    DWord()
    {
        **this = 0;
    }

    DWord(u32 val)
    {
        **this = val;
    }
};
static_assert(sizeof(DWord) == sizeof(u32), "Bad DWord Struct");
static_assert(offsetof(DWord, B._0) == 0, "Bad DWord Struct");
static_assert(offsetof(DWord, B._1) == 1, "Bad DWord Struct");
static_assert(offsetof(DWord, B._2) == 2, "Bad DWord Struct");
static_assert(offsetof(DWord, B._3) == 3, "Bad DWord Struct");
static_assert(offsetof(DWord, W._0) == 0, "Bad DWord Struct");
static_assert(offsetof(DWord, W._1) == 2, "Bad DWord Struct");

#ifdef _SP
#undef _SP
#endif

#ifdef _stat
#undef _stat
#endif