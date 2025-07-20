#include "structures/OceanWreck.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "structures/data/Wrecks.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

bool tryPlaceWreck(int x, int y, TileBuffer &wreck, World &world)
{
    int surfaceLeft = y;
    int rightX = x + wreck.getWidth() - 1;
    int surfaceRight =
        scanWhileEmpty({rightX, world.getSurfaceLevel(rightX)}, {0, 1}, world)
            .y +
        1;
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::empty, TileID::sand, TileID::coralstone, TileID::honey});
    for (y += 1 - wreck.getHeight(); y <= surfaceLeft; ++y) {
        int jLeft = surfaceLeft - y;
        int jRight = surfaceRight - y;
        if (jLeft < 0 || jRight < 0 || jLeft >= wreck.getHeight() ||
            jRight >= wreck.getHeight() ||
            wreck.getTile(0, jLeft).blockPaint != Paint::red ||
            wreck.getTile(wreck.getWidth() - 1, jRight).blockPaint !=
                Paint::red ||
            !world.regionPasses(
                x,
                y,
                wreck.getWidth(),
                wreck.getHeight(),
                [&clearableTiles](Tile &tile) {
                    return !tile.guarded &&
                           clearableTiles.contains(tile.blockID);
                })) {
            continue;
        }
        world.placeBuffer(x, y, wreck);
        return true;
    }
    return false;
}

void genOceanWreck(Random &rnd, World &world)
{
    std::cout << "Dropping debris\n";
    std::vector<int> wrecks(Data::wrecks.begin(), Data::wrecks.end());
    std::shuffle(wrecks.begin(), wrecks.end(), rnd.getPRNG());
    wrecks.resize(rnd.getInt(2, 4));
    std::vector<int> locations(150);
    std::iota(
        locations.begin(),
        locations.end(),
        world.oceanCaveCenter > world.getWidth() / 2 ? 50
                                                     : world.getWidth() - 200);
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    auto locItr = locations.begin();
    for (int wreckId : wrecks) {
        TileBuffer wreck = Data::getWreck(wreckId, world.getFramedTiles());
        for (; locItr != locations.end(); ++locItr) {
            int x = *locItr;
            if (tryPlaceWreck(
                    x,
                    scanWhileEmpty({x, world.getSurfaceLevel(x)}, {0, 1}, world)
                            .y +
                        1,
                    wreck,
                    world)) {
                ++locItr;
                break;
            }
        }
    }
}
