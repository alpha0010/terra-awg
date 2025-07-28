#ifndef PREFIX_H
#define PREFIX_H

#include <vector>

namespace Prefix
{
enum {
    none = 0,
    large = 1,
    massive = 2,
    dangerous = 3,
    savage = 4,
    sharp = 5,
    pointy = 6,
    bulky = 12,
    heavy = 14,
    light = 15,
    sighted = 16,
    rapid = 17,
    hasty_ranged = 18,
    intimidating = 19,
    deadly_ranged = 20,
    staunch = 21,
    powerful = 25,
    mystic = 26,
    adept = 27,
    masterful = 28,
    intense = 32,
    taboo = 33,
    celestial = 34,
    furious = 35,
    keen = 36,
    superior = 37,
    forceful = 38,
    quick_common = 42,
    deadly_common = 43,
    agile = 44,
    nimble = 45,
    murderous = 46,
    nasty = 51,
    manic = 52,
    hurtful = 53,
    strong = 54,
    unpleasant = 55,
    ruthless = 57,
    frenzying = 58,
    godly = 59,
    demonic = 60,
    zealous = 61,
    hard = 62,
    guarding = 63,
    armored = 64,
    warding = 65,
    arcane = 66,
    precise = 67,
    lucky = 68,
    jagged = 69,
    spiked = 70,
    angry = 71,
    menacing = 72,
    brisk = 73,
    fleeting = 74,
    hasty_accessory = 75,
    quick_accessory = 76,
    wild = 77,
    rash = 78,
    intrepid = 79,
    violent = 80,
    legendary = 81,
    unreal = 82,
    mythical = 83
};
}

namespace PrefixSet
{
void initCelebration();

extern std::vector<int> accessory;
extern std::vector<int> universal;
extern std::vector<int> common;
extern std::vector<int> melee;
extern std::vector<int> ranged;
extern std::vector<int> magic;
extern std::vector<int> magiclownokb;
} // namespace PrefixSet

#endif // PREFIX_H
