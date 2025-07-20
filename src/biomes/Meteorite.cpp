#include "biomes/Meteorite.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/Dungeon.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <iostream>

inline constexpr auto emptyBlocks = frozen::make_set<int>(
    {TileID::empty,
     TileID::livingWood,
     TileID::leaf,
     TileID::livingMahogany,
     TileID::mahoganyLeaf,
     TileID::meteorite});

void placeMeteorite(int x, int y, Random &rnd, World &world)
{
    int size = world.conf.meteoriteSize * rnd.getDouble(26, 30);
    int wallID = rnd.select(WallVariants::underworld);
    for (int i = -size; i < size; ++i) {
        for (int j = -size; j < size; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            double dist = std::hypot(i, j) / size;
            double threshold = 1.8 * (dist - 0.2) * (0.97 - dist);
            if ((std::abs(i) > 4 || j > 0) &&
                std::abs(rnd.getFineNoise(x + 2 * i, y + 2 * j)) < threshold) {
                if (tile.blockID != TileID::empty &&
                    (dist < 0.7 || fnv1a32pt(x + i, y + j) % 13 < 4)) {
                    if (j < 2 && emptyBlocks.contains(tile.blockID)) {
                        tile = {};
                    } else {
                        tile.blockID = TileID::meteorite;
                        tile.wallID = wallID;
                    }
                }
            } else if (dist < 0.6) {
                if (tile.blockID != TileID::empty &&
                    (dist < 0.35 || fnv1a32pt(x + i, y + j) % 13 < 8)) {
                    tile.blockID = TileID::empty;
                    if (j > size / 4) {
                        tile.liquid = Liquid::lava;
                    }
                }
                if (tile.wallID != WallID::empty) {
                    tile.wallID =
                        (tile.blockID != TileID::empty ||
                         std::abs(rnd.getFineNoise(3 * (x + i), 3 * (y + j))) >
                             0.06)
                            ? wallID
                            : WallID::empty;
                }
            }
        }
    }
    world.queuedTreasures.emplace_back([x, y, size](Random &, World &world) {
        double threshold = 0.85 * size;
        for (int i = -size; i < size; ++i) {
            for (int j = -size; j < size; ++j) {
                if (std::hypot(i, j) < threshold) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.liquid == Liquid::water) {
                        tile.liquid = Liquid::none;
                    }
                }
            }
        }
    });
}

void genMeteorite(Random &rnd, World &world)
{
    if (world.conf.meteorites <= 0) {
        return;
    }
    std::cout << "Bombarding surface\n";
    rnd.shuffleNoise();
    std::array avoidPoints{
        world.spawn.x,
        world.surfaceEvilCenter,
        computeDungeonCenter(world),
    };
    int buffer = 0.04 * world.getWidth();
    for (int placed = 0, tries = 100 * world.conf.meteorites;
         placed < world.conf.meteorites && tries > 0;
         --tries) {
        int x = rnd.getInt(375, world.getWidth() - 375);
        for (int col : avoidPoints) {
            if (std::abs(col - x) < buffer) {
                x = -1;
                break;
            }
        }
        if (x == -1) {
            continue;
        }
        int y = scanWhileEmpty({x, world.getSurfaceLevel(x)}, {0, 1}, world).y +
                rnd.getInt(-1, 5);
        int numEmpty = 0;
        if (y < world.getUndergroundLevel() &&
            world.regionPasses(
                x - 15,
                y - 8,
                30,
                23,
                [&numEmpty, &world](Tile &tile) {
                    if (emptyBlocks.contains(tile.blockID) ||
                        (world.conf.hiveQueen && tile.flag == Flag::none)) {
                        ++numEmpty;
                    }
                    return numEmpty < 90;
                })) {
            placeMeteorite(x, y, rnd, world);
            ++placed;
        }
    }
}
