#include "structures/Outline.h"

#include "Config.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <iostream>

inline constexpr auto embeddedTraps = frozen::make_set<int>({
    TileID::boulder,
    TileID::bouncyBoulder,
    TileID::explosives,
    TileID::friendlyBoulder,
    TileID::ghoulder,
    TileID::lavaBoulder,
    TileID::lifeCrystalBoulder,
    TileID::rainbowBoulder,
    TileID::rollingCactus,
    TileID::spiderBoulder,
});

inline constexpr auto oreBacklights = frozen::make_map<int, int>({
    {TileID::copperOre, WallID::Safe::amberGemspark},
    {TileID::tinOre, WallID::Safe::topazGemspark},
    {TileID::ironOre, WallID::Safe::diamondGemspark},
    {TileID::leadOre, WallID::Safe::sapphireGemspark},
    {TileID::silverOre, WallID::Safe::diamondGemspark},
    {TileID::tungstenOre, WallID::Safe::emeraldGemspark},
    {TileID::goldOre, WallID::Safe::topazGemspark},
    {TileID::platinumOre, WallID::Safe::diamondGemspark},
    {TileID::hellstone, WallID::Safe::rubyGemspark},
    {TileID::cobaltOre, WallID::Safe::sapphireGemspark},
    {TileID::palladiumOre, WallID::Safe::rubyGemspark},
    {TileID::mythrilOre, WallID::Safe::emeraldGemspark},
    {TileID::orichalcumOre, WallID::Safe::amethystGemspark},
    {TileID::adamantiteOre, WallID::Safe::rubyGemspark},
    {TileID::titaniumOre, WallID::Safe::diamondGemspark},
    {TileID::chlorophyteOre, WallID::Safe::emeraldGemspark},
});

void genGlobalOutline(World &world)
{
    std::cout << "Exposing details\n";
    bool illuminate = world.conf.fadedMemories < 0.001;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [illuminate, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (world.conf.celebration &&
                    world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                        return !tile.echoCoatBlock &&
                               oreBacklights.contains(tile.blockID);
                    })) {
                    tile.wallID = oreBacklights.at(tile.blockID);
                    tile.wallPaint = Paint::none;
                    continue;
                }
                if (!isSolidBlock(tile.blockID) || tile.actuated ||
                    (!illuminate && !tile.echoCoatBlock)) {
                    continue;
                }
                if (world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                        return isSolidBlock(tile.blockID) ||
                               embeddedTraps.contains(tile.blockID);
                    })) {
                    continue;
                }
                if (illuminate) {
                    tile.illuminantBlock = true;
                } else {
                    tile.echoCoatBlock = false;
                    tile.blockPaint = Paint::negative;
                }
            }
        });
}
