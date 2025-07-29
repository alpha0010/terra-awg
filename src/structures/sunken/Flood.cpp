#include "structures/sunken/Flood.h"

#include "Config.h"
#include "World.h"
#include "ids/Paint.h"
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
    if (world.getTile(startX, startY).liquid != Liquid::none) {
        return;
    }
    constexpr auto hardenTiles = frozen::make_set<int>(
        {TileID::empty,
         TileID::ashTree,
         TileID::tree,
         TileID::minecartTrack,
         TileID::platform,
         TileID::woodenBeam});
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
            !skipWalls.contains(tile.wallID) &&
            (tile.liquid == Liquid::none ||
             (tile.liquid != Liquid::water && tile.actuated))) {
            if (y + 10 > world.getUnderworldLevel() &&
                hardenTiles.contains(tile.blockID)) {
                int mixBlock = TileID::ash;
                if (world.conf.biomes != BiomeLayout::columns) {
                    switch (world.getBiome(x, y).active) {
                    case Biome::forest:
                        mixBlock = TileID::stone;
                        break;
                    case Biome::desert:
                        mixBlock = TileID::sandstone;
                        break;
                    case Biome::jungle:
                        mixBlock = TileID::mud;
                        break;
                    default:
                        break;
                    }
                }
                tile.blockID =
                    fnv1a32pt(x, y) % 7 < 2 ? mixBlock : TileID::obsidian;
                tile.frameX = 0;
                tile.frameY = 0;
                tile.blockPaint = Paint::none;
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
        } else if (
            tile.liquid == Liquid::lava &&
            (tile.blockID == TileID::empty ||
             tile.blockID == TileID::woodenBeam)) {
            tile.blockID = TileID::obsidian;
            tile.liquid = Liquid::none;
        }
    }
}

void floodBubbleInit(Point center, int size, World &world)
{
    for (int i = -size; i < size; ++i) {
        for (int j = -size; j < size; ++j) {
            if (std::hypot(i, j) < size) {
                Tile &tile = world.getTile(center + Point{i, j});
                if (tile.blockID == TileID::empty &&
                    !skipWalls.contains(tile.wallID)) {
                    tile.blockID = TileID::bubble;
                }
            }
        }
    }
}

void floodBubbleFinalize(Point center, int size, World &world)
{
    for (int i = -size; i < size; ++i) {
        for (int j = -size; j < size; ++j) {
            if (std::hypot(i, j) < size) {
                Tile &tile = world.getTile(center + Point{i, j});
                tile.liquid = Liquid::none;
                if (tile.blockID == TileID::bubble) {
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
}

void genFlood(World &world)
{
    std::cout << "Flooding\n";
    int probeDelta = world.conf.shattered ? 120 : 130;
    int floodLevel =
        std::max(
            {world.getSurfaceLevel(world.getWidth() / 2 - probeDelta),
             world.getSurfaceLevel(world.getWidth() / 2),
             world.getSurfaceLevel(world.getWidth() / 2 + probeDelta)}) +
        (world.conf.shattered ? 3 : 0);
    if (floodLevel < world.dungeon.y + 4) {
        floodBubbleInit(world.dungeon, 85, world);
    }
    bool shouldBubbleSpawn =
        floodLevel < world.spawn.y - 2 &&
        world.spawn.y < (world.conf.shattered
                             ? world.getUnderworldLevel()
                             : world.getUndergroundLevel() + 30);
    if (shouldBubbleSpawn) {
        floodBubbleInit(world.spawn, 45, world);
    }
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = floodLevel; y < world.getUndergroundLevel() + 30; ++y) {
            floodFill(x, y, floodLevel, world);
        }
    }
    if (floodLevel < world.dungeon.y + 4) {
        floodBubbleFinalize(world.dungeon, 82, world);
    }
    if (shouldBubbleSpawn) {
        floodBubbleFinalize(world.spawn, 42, world);
    }
}
