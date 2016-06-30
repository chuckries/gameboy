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
    virtual bool Init();

public:
    MBC_IDENTIFIER MBCID;
    bool HasRam;
    bool HasSave;

public:
    u8 operator [](int i) const
    {
        return _rom[i];
    }

protected:
    virtual bool LoadFromFile() = 0;

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

class MbcRomOnly
{
public:
    MbcRomOnly(const Rom& rom);
    virtual ~MbcRomOnly();

    virtual u8 LoadRom(u16 addr);
    virtual void StoreRom(u16 addr, u8 val);
    virtual u8 LoadRam(u16 addr);
    virtual void StoreRam(u16 addr, u8 val);

protected:
    const Rom& _rom;

    u32 _romOffset;
    u32 _ramOffset;

    bool _ramEnabled;
};

class Mbc1 : public MbcRomOnly
{
public:
    Mbc1(const Rom& rom);
    virtual ~Mbc1();

    virtual void StoreRom(u16 addr, u8 val);

protected:
    virtual void CalculateOffsets();

protected:
    u8 _reg2000;
    u8 _reg4000;
    bool _reg6000;
};

class Cart
{
public:
    Cart(const Gameboy& gameboy);
    virtual ~Cart();

    void Init(std::unique_ptr<Rom> rom);
    void UnInit();

    u8 LoadRom(u16 addr);
    void StoreRom(u16 addr, u8 val);
    u8 LoadRam(u16 addr);
    void StoreRam(u16 addr, u8 val);

private:
    const Gameboy& _gameboy;
    std::unique_ptr<Rom> _rom;

private:
    MBC_IDENTIFIER _mbcId;
    std::unique_ptr<Mbc1> _mbc1;
};