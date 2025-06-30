#include "Desert.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>
#include <map>

void genDesert(Random &rnd, World &world)
{
    std::cout << "Desertification\n";
    rnd.shuffleNoise();
    int noiseShuffleX = rnd.getInt(0, world.getWidth());
    int noiseShuffleY = rnd.getInt(0, world.getHeight());
    double center = world.desertCenter;
    double scanDist = world.conf.desertSize * 0.08 * world.getWidth();
    double desertFloor =
        (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5;
    std::map<int, int> sandWalls{
        {WallID::Unsafe::marble, WallID::Unsafe::marble},
        {WallID::Safe::cloud, WallID::Safe::cloud}};
    for (int wallId : WallVariants::dirt) {
        sandWalls[wallId] = rnd.select(
            {WallID::Unsafe::sandstone, WallID::Unsafe::hardenedSand});
    }
    fillLargeWallGaps(
        {center - 0.9 * scanDist, 0.95 * world.getUndergroundLevel()},
        {center + 0.9 * scanDist, 0.96 * desertFloor},
        rnd,
        world);
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center,
         desertFloor,
         noiseShuffleX,
         noiseShuffleY,
         &sandWalls,
         &rnd,
         &world](int x) {
            for (int y = 0; y < world.getUnderworldLevel(); ++y) {
                double threshold = std::max(
                    std::abs(x - center) / 100.0 -
                        (world.conf.desertSize * world.getWidth() / 1700.0),
                    15 * (y - desertFloor) / world.getHeight());
                if (rnd.getCoarseNoise(x, y) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x, y);
                switch (tile.blockID) {
                case TileID::dirt:
                    tile.blockID = TileID::sand;
                    break;
                case TileID::ice:
                    if (rnd.getFineNoise(x, y) > -0.02) {
                        break;
                    }
                    [[fallthrough]];
                case TileID::stone:
                case TileID::smoothMarble:
                    tile.blockID = TileID::sandstone;
                    if (y > world.getCavernLevel()) {
                        if (std::abs(rnd.getCoarseNoise(x, y) + 0.23) < 0.04) {
                            tile.blockID = TileID::sand;
                        } else if (
                            std::abs(rnd.getCoarseNoise(x, y) - 0.23) < 0.04) {
                            tile.blockID = TileID::hardenedSand;
                        }
                    }
                    break;
                case TileID::clay:
                case TileID::mud:
                    tile.blockID = TileID::desertFossil;
                    break;
                case TileID::sand:
                    tile.blockID =
                        y > world.getCavernLevel()        ? TileID::desertFossil
                        : y > world.getUndergroundLevel() ? TileID::hardenedSand
                                                          : TileID::sand;
                    break;
                default:
                    break;
                }
                if (tile.blockID == TileID::sandstone) {
                    tile.wallID = WallID::Unsafe::sandstone;
                } else if (
                    tile.blockID == TileID::sand ||
                    tile.blockID == TileID::hardenedSand) {
                    tile.wallID = WallID::Unsafe::hardenedSand;
                } else {
                    tile.wallID = sandWalls[tile.wallID];
                }

                threshold = std::max(
                    threshold,
                    3.0 * (world.getUndergroundLevel() - y) /
                        world.getHeight());
                bool shouldClear =
                    std::abs(rnd.getBlurNoise(x, 5 * y)) >
                        std::max(threshold + 1.2, 0.4) &&
                    rnd.getFineNoise(noiseShuffleX + x, noiseShuffleY + y) >
                        -0.3;
                if (shouldClear && (tile.blockID == TileID::sandstone ||
                                    ((tile.blockID == TileID::sand ||
                                      tile.blockID == TileID::hardenedSand) &&
                                     rnd.getFineNoise(x, y) > 0))) {
                    tile.blockID = TileID::empty;
                }
                if (tile.wallID == WallID::empty &&
                    y > world.getCavernLevel() &&
                    rnd.getFineNoise(x, y) < 0.5) {
                    tile.wallID = WallID::Unsafe::sandstone;
                }
            }
        });
}
