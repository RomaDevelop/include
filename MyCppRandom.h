//---------------------------------------------------------------------------
#ifndef MyCppRandom_H
#define MyCppRandom_H
//---------------------------------------------------------------------------
#include <random>
//---------------------------------------------------------------------------

struct MyCppRandom
{
    inline static int Get(int from, int to);
};

//---------------------------------------------------------------------------

int MyCppRandom::Get(int from, int to)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(from, to);
    return distrib(gen);
}

//---------------------------------------------------------------------------
#endif
