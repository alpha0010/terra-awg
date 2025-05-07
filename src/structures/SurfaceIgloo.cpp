#include "structures/SurfaceIgloo.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Igloos.h"
#include <algorithm>
#include <iostream>
#include <set>

bool canPlaceIglooAt(int x, int y, TileBuffer &igloo, World &world)
{
    int surfaceLeft =
        scanWhileEmpty({x, world.getSurfaceLevel(x)}, {0, 1}, world).second - y;
    int surfaceRight = scanWhileEmpty(
                           {x + igloo.getWidth() - 1,
                            world.getSurfaceLevel(x + igloo.getWidth() - 1)},
                           {0, 1},
                           world)
                           .second -
                       y;
    if (surfaceLeft < 0 || surfaceRight < 0 ||
        surfaceLeft >= igloo.getHeight() || surfaceRight >= igloo.getHeight() ||
        igloo.getTile(0, surfaceLeft).blockPaint != Paint::red ||
        igloo.getTile(igloo.getWidth() - 1, surfaceRight).blockPaint !=
            Paint::red) {
        return false;
    }
    std::set<int> clearableTiles{
        TileID::empty,
        TileID::snow,
        TileID::ice,
        TileID::copperOre,
        TileID::tinOre};
    return world.regionPasses(
        x,
        y,
        igloo.getWidth(),
        igloo.getHeight(),
        [&clearableTiles](Tile &tile) {
            return clearableTiles.contains(tile.blockID);
        });
}

bool placeIgloo(Point pt, TileBuffer &igloo, Random &rnd, World &world)
{
    int minOpenHeight = igloo.getHeight();
    int maxOpenHeight = -1;
    for (int j = 0; j < igloo.getHeight(); ++j) {
        if (igloo.getTile(0, j).blockPaint == Paint::red) {
            minOpenHeight = std::min(j, minOpenHeight);
            maxOpenHeight = std::max(j, maxOpenHeight);
        }
    }
    int x = pt.first;
    int y = pt.second + std::midpoint(minOpenHeight, maxOpenHeight) -
            igloo.getHeight();
    for (int offsetSwap = 0; offsetSwap < 2 * igloo.getHeight(); ++offsetSwap) {
        int offset = offsetSwap / 2;
        if (offsetSwap % 2 == 1) {
            offset = -offset;
        }
        if (canPlaceIglooAt(x, y + offset, igloo, world)) {
            y += offset;
            for (int i = 0; i < igloo.getWidth(); ++i) {
                for (int j = 0; j < igloo.getHeight(); ++j) {
                    Tile &iglooTile = igloo.getTile(i, j);
                    if (iglooTile.blockID == TileID::cloud) {
                        continue;
                    }
                    Tile &tile = world.getTile(x + i, y + j);
                    if (iglooTile.wallID == WallID::empty &&
                        iglooTile.blockID != TileID::empty) {
                        iglooTile.wallID = tile.wallID;
                    }
                    tile = iglooTile;
                    tile.guarded = true;
                    if (tile.blockID == TileID::chest &&
                        tile.frameX % 36 == 0 && tile.frameY == 0) {
                        if (tile.frameX == 180) {
                            fillBarrel(
                                world.registerStorage(x + i, y + j),
                                rnd);
                        } else {
                            fillSurfaceFrozenChest(
                                world.registerStorage(x + i, y + j),
                                rnd,
                                world);
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void genIgloo(Random &rnd, World &world)
{
    std::cout << "Piling snow\n";
    int scanDist = 0.05 * world.getWidth();
    std::vector<int> locations;
    int spawnX = world.getWidth() / 2;
    for (int x = world.snowCenter - scanDist; x < world.snowCenter + scanDist;
         ++x) {
        if (std::abs(x - spawnX) > 100) {
            locations.push_back(x);
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    TileBuffer igloo =
        Data::getIgloo(rnd.select(Data::igloos), world.getFramedTiles());
    for (int x : locations) {
        if (placeIgloo(
                scanWhileEmpty({x, world.getSurfaceLevel(x)}, {0, 1}, world),
                igloo,
                rnd,
                world)) {
            break;
        }
    }
}
