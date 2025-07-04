#include "structures/sunken/Flood.h"

#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <cmath>
#include <iostream>

inline constexpr auto skipWalls = frozen::make_set<int>({
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

void floodFill(int startX, int startY, int minY, World &world)
{
    std::vector<Point> locations{{startX, startY}};
    while (!locations.empty()) {
        auto [x, y] = locations.back();
        locations.pop_back();
        if (x < 0 || x >= world.getWidth() || y < minY ||
            y > world.getUnderworldLevel()) {
            continue;
        }
        Tile &tile = world.getTile(x, y);
        if ((!isSolidBlock(tile.blockID) || tile.actuated) &&
            tile.blockID != TileID::bubble && tile.blockID != TileID::door &&
            !skipWalls.contains(tile.wallID) && tile.liquid == Liquid::none) {
            if (y + 10 > world.getUnderworldLevel() &&
                (tile.blockID == TileID::empty ||
                 tile.blockID == TileID::minecartTrack)) {
                tile.blockID = TileID::obsidian;
                tile.frameX = 0;
                tile.frameY = 0;
            } else {
                tile.liquid = Liquid::water;
            }
            if (y < world.getCavernLevel()) {
                for (auto [i, j] :
                     {std::pair{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                    locations.emplace_back(x + i, y + j);
                }
            } else {
                for (auto [i, j] : {std::pair{-1, 0}, {1, 0}, {0, 1}}) {
                    locations.emplace_back(x + i, y + j);
                }
            }
        }
    }
}

void genFlood(World &world)
{
    std::cout << "Flooding\n";
    int floodLevel = std::max(
        {world.getSurfaceLevel(world.getWidth() / 2 - 130),
         world.getSurfaceLevel(world.getWidth() / 2),
         world.getSurfaceLevel(world.getWidth() / 2 + 130)});
    if (floodLevel < world.dungeonY + 2) {
        for (int i = -85; i < 85; ++i) {
            for (int j = -85; j < 85; ++j) {
                if (std::hypot(i, j) < 85) {
                    Tile &tile =
                        world.getTile(world.dungeonX + i, world.dungeonY + j);
                    if (tile.blockID == TileID::empty &&
                        !skipWalls.contains(tile.wallID)) {
                        tile.blockID = TileID::bubble;
                    }
                }
            }
        }
    }
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = floodLevel; y < world.getUndergroundLevel() + 30; ++y) {
            floodFill(x, y, floodLevel, world);
        }
    }
    if (floodLevel < world.dungeonY + 2) {
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
}
