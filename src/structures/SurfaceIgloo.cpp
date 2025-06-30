#include "structures/SurfaceIgloo.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Igloos.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

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
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::empty,
         TileID::snow,
         TileID::ice,
         TileID::copperOre,
         TileID::tinOre,
         TileID::cobaltOre,
         TileID::palladiumOre});
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
            std::vector<std::tuple<int, int, int>> wallMerges;
            for (int i = 0; i < igloo.getWidth(); ++i) {
                int maxClearY = std::lerp(
                                    world.getSurfaceLevel(x),
                                    world.getSurfaceLevel(x + igloo.getWidth()),
                                    static_cast<double>(i) / igloo.getWidth()) -
                                1;
                for (int cY = world.getSurfaceLevel(x + i) - 1; cY < maxClearY;
                     ++cY) {
                    world.getTile(x + i, cY) = {};
                }
                for (int j = 0; j < igloo.getHeight(); ++j) {
                    Tile &iglooTile = igloo.getTile(i, j);
                    if (iglooTile.blockID == TileID::cloud) {
                        continue;
                    }
                    Tile &tile = world.getTile(x + i, y + j);
                    if (iglooTile.wallID == WallID::empty &&
                        isSolidBlock(iglooTile.blockID) &&
                        tile.wallID != WallID::empty) {
                        wallMerges.emplace_back(x + i, y + j, tile.wallID);
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
            for (auto [wX, wY, wallID] : wallMerges) {
                if (!world.isExposed(wX, wY)) {
                    world.getTile(wX, wY).wallID = wallID;
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
    std::vector<int> locations;
    int spawnX = world.getWidth() / 2;
    if (world.conf.patches) {
        for (int x = 350; x < world.getWidth() - 350; ++x) {
            if (std::abs(x - spawnX) > 100 &&
                world.getBiome(x, world.getSurfaceLevel(x)).snow > 0.99) {
                locations.push_back(x);
            }
        }
    } else {
        int scanDist = world.conf.snowSize * 0.05 * world.getWidth();
        for (int x = world.snowCenter - scanDist;
             x < world.snowCenter + scanDist;
             ++x) {
            if (std::abs(x - spawnX) > 100) {
                locations.push_back(x);
            }
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
