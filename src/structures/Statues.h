#ifndef STATUES_H
#define STATUES_H

#include <array>

class World;

namespace Statue
{
enum {
    armedZombie = 684'054,
    bat = 252'000,
    bloodZombie = 720'054,
    boneSkeleton = 432'054,
    chest = 972'000,
    corrupt = 1080'000,
    crab = 648'000,
    drippler = 360'054,
    eyeball = 1260'000,
    goblin = 180'000,
    graniteGolem = 648'054,
    harpy = 540'054,
    hoplite = 612'054,
    hornet = 576'000,
    imp = 468'000,
    jellyfish = 828'000,
    medusa = 504'054,
    pigron = 576'054,
    piranha = 1512'000,
    shark = 1800'000,
    skeleton = 360'000,
    slime = 144'000,
    undeadViking = 468'054,
    unicorn = 324'054,
    wallCreeper = 288'054,
    wraith = 396'054,
    king = 1440'000,
    queen = 1476'000,
    bomb = 612'000,
    heart = 1332'000,
    star = 72'000,
    mushroom = 1224'000,
};

}

namespace StatueVariants
{

inline std::array const enemy = {
    Statue::armedZombie,  Statue::bat,          Statue::bloodZombie,
    Statue::boneSkeleton, Statue::chest,        Statue::corrupt,
    Statue::crab,         Statue::drippler,     Statue::eyeball,
    Statue::goblin,       Statue::graniteGolem, Statue::harpy,
    Statue::hoplite,      Statue::hornet,       Statue::imp,
    Statue::jellyfish,    Statue::medusa,       Statue::pigron,
    Statue::piranha,      Statue::shark,        Statue::skeleton,
    Statue::slime,        Statue::undeadViking, Statue::unicorn,
    Statue::wallCreeper,  Statue::wraith};

inline std::array const utility =
    {Statue::king, Statue::queen, Statue::bomb, Statue::heart, Statue::star};

} // namespace StatueVariants

void placeStatue(int x, int y, int style, World &world);

#endif // STATUES_H
