#include "structures/Vines.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include <iostream>
#include <map>

void genVines(Random &rnd, World &world)
{
    std::cout << "Growing vines\n";
    std::map<int, int> vineTypes{
        {TileID::grass, TileID::vines},
        {TileID::leaf, TileID::vines},
        {TileID::jungleGrass, TileID::jungleVines},
        {TileID::mahoganyLeaf, TileID::vineRope},
        {TileID::corruptGrass, TileID::corruptVines},
        {TileID::corruptJungleGrass, TileID::corruptVines},
        {TileID::crimsonGrass, TileID::crimsonVines},
        {TileID::crimsonJungleGrass, TileID::crimsonVines},
        {TileID::mushroomGrass, TileID::mushroomVines},
        {TileID::ashGrass, TileID::ashVines}};
    std::map<int, int> dropperTypes{
        {TileID::dirt, TileID::waterDrip},
        {TileID::stone, TileID::waterDrip},
        {TileID::jungleGrass, TileID::waterDrip},
        {TileID::cloud, TileID::waterDrip},
        {TileID::rainCloud, TileID::waterDrip},
        {TileID::granite, TileID::waterDrip},
        {TileID::marble, TileID::waterDrip},
        {TileID::livingMahogany, TileID::waterDrip},
        {TileID::mahoganyLeaf, TileID::waterDrip},
        {TileID::lihzahrdBrick, TileID::waterDrip},
        {TileID::ash, TileID::lavaDrip},
        {TileID::ashGrass, TileID::lavaDrip},
        {TileID::obsidianBrick, TileID::lavaDrip},
        {TileID::hellstoneBrick, TileID::lavaDrip},
        {TileID::sand, TileID::sandDrip},
        {TileID::hardenedSand, TileID::sandDrip},
        {TileID::sandstone, TileID::sandDrip},
        {TileID::ebonsand, TileID::sandDrip},
        {TileID::hardenedEbonsand, TileID::sandDrip},
        {TileID::ebonsandstone, TileID::sandDrip},
        {TileID::crimsand, TileID::sandDrip},
        {TileID::hardenedCrimsand, TileID::sandDrip},
        {TileID::crimsandstone, TileID::sandDrip},
        {TileID::hive, TileID::honeyDrip}};
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    parallelFor(std::views::iota(0, world.getWidth()), [&](int x) {
        int vine = TileID::empty;
        int dropper = TileID::empty;
        int vineLen = 0;
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            int randInt = 99999 * (1 + rnd.getFineNoise(x, y));
            if (vineLen > 0) {
                if (tile.blockID == TileID::empty &&
                    tile.liquid == Liquid::none) {
                    tile.blockID = vine;
                    if (vine == TileID::vineRope) {
                        tile.blockPaint = Paint::lime;
                    }
                    --vineLen;
                    continue;
                } else {
                    vineLen = 0;
                }
            } else if (
                dropper != TileID::empty && tile.blockID == TileID::empty &&
                tile.liquid == Liquid::none && randInt % 67 == 0) {
                tile.blockID = dropper == TileID::waterDrip && y > lavaLevel &&
                                       randInt % 5 != 0
                                   ? TileID::lavaDrip
                                   : dropper;
                dropper = TileID::empty;
                continue;
            }
            dropper = TileID::empty;
            if (tile.slope != Slope::none || tile.actuated) {
                continue;
            }
            auto vineItr = vineTypes.find(tile.blockID);
            if (vineItr == vineTypes.end() || randInt % 3 == 0) {
                auto dropperItr = dropperTypes.find(tile.blockID);
                if (dropperItr != dropperTypes.end()) {
                    dropper = dropperItr->second;
                }
                continue;
            }
            vine = vineItr->second;
            if (vine == TileID::vines && rnd.getCoarseNoise(x, y) > 0.12) {
                vine = TileID::flowerVines;
            }
            vineLen = 4 + randInt % 7;
        }
    });
}
