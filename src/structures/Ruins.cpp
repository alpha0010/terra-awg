#include "structures/Ruins.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/Platforms.h"
#include <iostream>
#include <set>

int makeCongruent(int val, int mod)
{
    return mod * (val / mod);
}

void genRuins(Random &rnd, World &world)
{
    std::cout << "Abandoning cities\n";
    int step = 6;
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int cityBase = makeCongruent(
        world.getUnderworldLevel() + 0.49 * underworldHeight,
        step);
    auto isInBuilding = [cityBase, step, underworldHeight, &rnd](int x, int y) {
        double threshold =
            1.6 * (cityBase - makeCongruent(y, step)) / underworldHeight - 0.2;
        return rnd.getFineNoise(
                   makeCongruent(x, step),
                   makeCongruent(y, step)) > threshold;
    };
    auto isRoomWall = [step, &rnd](int x, int y) {
        return static_cast<int>(
                   99999 * (1 + rnd.getFineNoise(x, makeCongruent(y, step)))) %
                   23 ==
               0;
    };
    auto isPlatform = [step, &rnd](int x, int y) {
        return static_cast<int>(
                   99999 * (1 + rnd.getFineNoise(
                                    makeCongruent(x, step / 2) + step / 4,
                                    y))) %
                   11 ==
               0;
    };
    std::set<std::pair<int, int>> queuedPlatforms;
    for (int x = 0; x < world.getWidth(); ++x) {
        if (x % step == 0 && (rnd.getCoarseNoise(x, 0) < 0.15 ||
                              !world.regionPasses(
                                  x,
                                  world.getUnderworldLevel(),
                                  step,
                                  cityBase - world.getUnderworldLevel(),
                                  [](Tile &tile) { return !tile.guarded; }))) {
            x += step - 1;
            continue;
        } else if (isInBuilding(x, cityBase - 1)) {
            // Base.
            Tile &tile = world.getTile(x, cityBase);
            tile.blockID = TileID::obsidianBrick;
            tile.liquid = Liquid::none;
            for (int j = 1; j < 20; ++j) {
                double noise = rnd.getFineNoise(x, cityBase + j);
                if (noise > j / 10.0 - 1) {
                    Tile &baseTile = world.getTile(x, cityBase + j);
                    if (baseTile.blockID == TileID::empty) {
                        baseTile.blockID =
                            static_cast<int>(99999 * (1 + noise)) % 5 == 0
                                ? TileID::obsidianBrick
                                : TileID::ash;
                        baseTile.liquid = Liquid::none;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            continue;
        }
        for (int y = cityBase - 1; y > world.getUnderworldLevel(); --y) {
            if (!isInBuilding(x, y)) {
                break;
            }
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::lesion ||
                tile.blockID == TileID::flesh) {
                continue;
            }
            tile.liquid = Liquid::none;
            if (!isInBuilding(x - 1, y) ||
                (isInBuilding(x, y - 1) && !isInBuilding(x - 1, y - 1)) ||
                !isInBuilding(x + 1, y) ||
                (isInBuilding(x, y - 1) && !isInBuilding(x + 1, y - 1))) {
                // Outer wall.
                if (step - y % step == 3 &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            7 ==
                        0) {
                    world
                        .placeFramedTile(x, y, TileID::door, Variant::obsidian);
                } else {
                    tile.blockID = TileID::obsidianBrick;
                }
            } else if (
                isRoomWall(x, y) && !isRoomWall(x + 1, y) &&
                !isRoomWall(x + 2, y)) {
                // Room wall.
                if (step - y % step == 3 &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            2 ==
                        0) {
                    world
                        .placeFramedTile(x, y, TileID::door, Variant::obsidian);
                } else {
                    tile.blockID = TileID::obsidianBrick;
                }
                tile.wallID = WallID::Unsafe::obsidianBrick;
            } else if (!isInBuilding(x, y - 1)) {
                // Roof.
                tile.blockID = TileID::obsidianBrick;
                if (isPlatform(x, y)) {
                    queuedPlatforms.emplace(x, y);
                }
            } else if (y % step == 0) {
                // Room floor.
                tile.blockID = TileID::obsidianBrick;
                tile.wallID = WallID::Unsafe::obsidianBrick;
                if (isPlatform(x, y)) {
                    queuedPlatforms.emplace(x, y);
                }
            } else {
                // Room.
                tile.blockID = TileID::empty;
                tile.wallID = WallID::Unsafe::obsidianBrick;
            }
        }
    }
    std::erase_if(queuedPlatforms, [&world](const auto &pt) {
        return world.getTile(pt.first, pt.second - 1).blockID !=
                   TileID::empty ||
               world.getTile(pt.first, pt.second + 1).blockID != TileID::empty;
    });
    for (auto [x, y] : queuedPlatforms) {
        if (queuedPlatforms.contains({x - 1, y}) ||
            queuedPlatforms.contains({x + 1, y})) {
            placePlatform(x, y, Platform::obsidian, world);
        }
    }
}
