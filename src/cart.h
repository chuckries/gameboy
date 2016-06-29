#pragma once

class Gameboy;

enum MBC_IDENTIFIER
{
    MBC_ROM_ONLY = 0,
    MBC_1 = 1,
    MBC_2 = 2,
    MBC_3 = 3,
    MBC_4 = 4,

    MBC_UNKNOWN
};

class Rom
{
protected:
    Rom() : _rom(0) {}

public:
    virtual ~Rom() { }
    virtual bool LoadFromFile() = 0;

public:
    MBC_IDENTIFIER MBCID();

public:
    u8 operator [](int i) const
    {
        return _rom[i];
    }

protected:
    std::vector<u8> _rom;
};

class StdRom : public Rom
{
public:
    StdRom(const char* filPath);

    virtual bool LoadFromFile();

private:
    const char* _filePath;
};

class MBC1
{
public:
    MBC1(const Rom& rom);
    virtual ~MBC1();

    virtual u8 LoadRom(u16 addr);
    virtual void StoreRom(u16 addr, u8 val);
    //virtual u8 LoadRam(u16 addr);
    //virtual void StoreRam(u16 addr, u8 val);

private:
    const Rom& _rom;

    u16 _addrOffset;
};

class Cart
{
public:
    Cart(const Gameboy& gameboy);
    virtual ~Cart();

    void Init(std::unique_ptr<Rom> rom);
    void UnInit();

    u8 Load(u16 addr);
    void Store(u16 addr, u8 val);

private:
    const Gameboy& _gameboy;
    std::unique_ptr<Rom> _rom;

private:
    MBC_IDENTIFIER _mbcId;
    std::unique_ptr<MBC1> _mbc1;
};