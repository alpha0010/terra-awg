#include "structures/sunken/Flood.h"

#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <cmath>
#include <iostream>

void floodFill(int startX, int startY, int minY, World &world)
{
    std::vector<Point> locations{{startX, startY}};
    constexpr auto skipWalls = frozen::make_set<int>({
        WallID::Unsafe::blueBrick,
        WallID::Unsafe::blueSlab,
        WallID::Unsafe::blueTiled,
        WallID::Unsafe::greenBrick,
        WallID::Unsafe::greenSlab,
        WallID::Unsafe::greenTiled,
        WallID::Unsafe::pinkBrick,
        WallID::Unsafe::pinkSlab,
        WallID::Unsafe::pinkTiled,
        WallID::Unsafe::lihzahrdBrick,
    });
    while (!locations.empty()) {
        auto [x, y] = locations.back();
        locations.pop_back();
        if (x < 0 || x >= world.getWidth() || y < minY ||
            y > world.getUnderworldLevel()) {
            continue;
        }
        Tile &tile = world.getTile(x, y);
        if (!isSolidBlock(tile.blockID) && tile.blockID != TileID::bubble &&
            !skipWalls.contains(tile.wallID) && tile.liquid == Liquid::none) {
            tile.liquid = Liquid::water;
            for (auto [i, j] : {std::pair{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                locations.emplace_back(x + i, y + j);
            }
        }
    }
}

void genFlood(World &world)
{
    std::cout << "Flooding\n";
    for (int i = -85; i < 85; ++i) {
        for (int j = -85; j < 48; ++j) {
            if (std::hypot(i, j) < 85) {
                Tile &tile =
                    world.getTile(world.dungeonX + i, world.dungeonY + j);
                if (tile.blockID == TileID::empty) {
                    tile.blockID = TileID::bubble;
                }
            }
        }
    }
    int floodLevel = std::max(
        world.getSurfaceLevel(world.getWidth() / 2 - 130),
        world.getSurfaceLevel(world.getWidth() / 2 + 130));
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = floodLevel; y < world.getUndergroundLevel() + 30; ++y) {
            floodFill(x, y, floodLevel, world);
        }
    }
    for (int i = -82; i < 82; ++i) {
        for (int j = -82; j < 82; ++j) {
            if (std::hypot(i, j) < 82) {
                Tile &tile =
                    world.getTile(world.dungeonX + i, world.dungeonY + j);
                tile.liquid = Liquid::none;
                if (tile.blockID == TileID::bubble) {
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
}
