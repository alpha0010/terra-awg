#include "structures/Outline.h"

#include "Config.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "structures/StructureUtil.h"
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

void genGlobalOutline(World &world)
{
    std::cout << "PING.. Ping.. ping..\n";
    bool illuminate = world.conf.fadedMemories < 0.001;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [illuminate, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
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
