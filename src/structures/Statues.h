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
    anvil = 1152'000,
    axe = 1044'000,
    boomerang = 900'000,
    boot = 936'000,
    bow = 864'000,
    cross = 792'000,
    gargoyle = 504'000,
    gloom = 540'000,
    hammer = 684'000,
    pickaxe = 1188'000,
    pillar = 1296'000,
    pot = 1368'000,
    potion = 720'000,
    reaper = 396'000,
    shield = 216'000,
    spear = 756'000,
    sunflower = 1404'000,
    sword = 108'000,
    tree = 1116'000,
    woman = 432'000,
};

}

namespace StatueVariants
{

inline std::array const deco = {
    Statue::anvil,     Statue::axe,     Statue::boomerang, Statue::boot,
    Statue::bow,       Statue::cross,   Statue::gargoyle,  Statue::gloom,
    Statue::hammer,    Statue::pickaxe, Statue::pillar,    Statue::pot,
    Statue::potion,    Statue::reaper,  Statue::shield,    Statue::spear,
    Statue::sunflower, Statue::sword,   Statue::tree,      Statue::woman};

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
