#include "structures/OceanWreck.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "structures/data/Wrecks.h"
#include <algorithm>
#include <iostream>
#include <set>

bool tryPlaceWreck(int x, int y, TileBuffer &wreck, World &world)
{
    int surfaceLeft = y;
    int rightX = x + wreck.getWidth() - 1;
    int surfaceRight =
        scanWhileEmpty({rightX, world.getSurfaceLevel(rightX)}, {0, 1}, world)
            .second +
        1;
    std::set<int> clearableTiles{
        TileID::empty,
        TileID::sand,
        TileID::coralstone};
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
        for (int i = 0; i < wreck.getWidth(); ++i) {
            for (int j = 0; j < wreck.getHeight(); ++j) {
                Tile &wreckTile = wreck.getTile(i, j);
                if (wreckTile.blockID == TileID::cloud) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                tile = wreckTile;
                tile.guarded = tile.blockID != TileID::empty ||
                               tile.wallID != WallID::empty;
            }
        }
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
                            .second +
                        1,
                    wreck,
                    world)) {
                ++locItr;
                break;
            }
        }
    }
}
