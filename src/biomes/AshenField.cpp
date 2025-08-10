#include "AshenField.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

void genAshenField(Random &rnd, World &world)
{
    if (world.conf.spawn != SpawnPoint::ashen &&
        (world.conf.spawn != SpawnPoint::surface ||
         rnd.getDouble(0, 1) > 0.7)) {
        return;
    }
    double width = 100 + world.getWidth() / rnd.getInt(64, 85);
    if (world.conf.shattered) {
        width *= 0.82;
    }
    int minX = world.spawn.x - width;
    int maxX = world.spawn.x + width;
    int minY = std::min(
                   {world.spawn.y,
                    world.getSurfaceLevel(minX),
                    world.getSurfaceLevel(maxX)}) -
               20;
    int maxY = std::midpoint<double>(minY + width, world.getUndergroundLevel());
    constexpr auto avoidTiles = frozen::make_set<int>({
        TileID::snow,
        TileID::sandstone,
        TileID::granite,
        TileID::marble,
        TileID::jungleGrass,
        TileID::livingWood,
    });
    if (!world.regionPasses(
            minX,
            minY,
            maxX - minX,
            maxY - minY,
            [&avoidTiles](Tile &tile) {
                return !avoidTiles.contains(tile.blockID);
            })) {
        return;
    }
    std::cout << "Managing wildfire\n";
    rnd.shuffleNoise();
    std::map<int, int> underworldWalls;
    for (int wallId : WallVariants::dirt) {
        underworldWalls[wallId] = rnd.select(WallVariants::underworld);
    }
    std::map<int, int> stoneWalls;
    for (int wallId : WallVariants::dirt) {
        stoneWalls[wallId] = rnd.select(WallVariants::stone);
    }
    double height = maxY - world.spawn.y;
    for (int x = minX; x < maxX; ++x) {
        int surface = scanWhileEmpty({x, minY}, {0, 1}, world).y;
        surface = std::lerp(
            world.spawn.y,
            surface < world.getUndergroundLevel() ? surface
                                                  : world.getSurfaceLevel(x),
            std::abs(x - world.spawn.x) / width);
        for (int y = minY; y < maxY; ++y) {
            double threshold = std::hypot(
                (x - world.spawn.x) / width,
                (y - world.spawn.y) / height);
            if (rnd.getFineNoise(x, y) < 9 * threshold - 8) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            if (y <= surface) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                continue;
            }
            if (threshold > 0.2 && std::abs(rnd.getCoarseNoise(3 * x, 3 * y)) <
                                       std::min(1.0, 5.8 - 7 * threshold) *
                                           (0.05 + 0.3 * (y - minY) / height)) {
                tile.blockID = TileID::empty;
                tile.wallID = y > surface + 2 ? underworldWalls[tile.wallID]
                                              : WallID::empty;
                if (y > surface + 1) {
                    tile.liquid = Liquid::lava;
                }
                continue;
            }
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::grass:
                tile.blockID = TileID::ash;
                break;
            case TileID::empty:
            case TileID::stone:
            case TileID::mud:
            case TileID::sand:
            case TileID::clay:
                tile.blockID =
                    y > world.spawn.y + 10 ? TileID::obsidian : TileID::ash;
                break;
            default:
                break;
            }
            if (tile.blockID == TileID::ash) {
                if (y - 3 < surface &&
                    world.getTile(x, y - 1).blockID == TileID::empty) {
                    tile.blockID = TileID::ashGrass;
                } else {
                    tile.wallID = underworldWalls[tile.wallID];
                }
            } else if (tile.blockID == TileID::obsidian) {
                tile.wallID = stoneWalls[tile.wallID];
            }
        }
    }
}
