#include "biomes/jaggedRocks/Chasms.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include <iostream>
#include <numbers>

void genChasmAt(
    Pointf pt,
    double weight,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 1.1) {
        return;
    }
    Pointf delta{std::cos(angle), std::sin(angle)};
    double turnPoint = 0.75 * weight;
    for (; weight > turnPoint; pt += delta, weight -= 0.03) {
        if (pt.x < 0 || pt.y < 0 || pt.x > world.getWidth() ||
            pt.y > world.getHeight()) {
            return;
        }
        for (double i = -weight; i < weight; ++i) {
            for (double j = -weight; j < weight; ++j) {
                double threshold = 2 - 3 * std::hypot(i, j) / weight;
                if (rnd.getFineNoise(pt.x + i, pt.y + j) < threshold) {
                    Tile &tile = world.getTile(pt.x + i, pt.y + j);
                    tile.wireRed = true;
                }
            }
        }
        if (rnd.getInt(0, 125) == 0) {
            double offset =
                rnd.getDouble(-std::numbers::pi / 2, std::numbers::pi / 2);
            offset += offset > 0 ? std::numbers::pi / 4 : -std::numbers::pi / 4;
            genChasmAt(pt, 0.9 * weight - 0.5, angle + offset, rnd, world);
        }
    }
    genChasmAt(
        pt,
        weight,
        angle + rnd.getDouble(-std::numbers::pi / 7, std::numbers::pi / 7),
        rnd,
        world);
}

void genChasms(Random &rnd, World &world)
{
    std::cout << "Fracturing land\n";
    rnd.shuffleNoise();
    std::vector<Point> usedLocations{
        {125, world.getSurfaceLevel(125)},
        {world.getWidth() - 125,
         world.getSurfaceLevel(world.getWidth() - 125)}};
    int numChasms = world.getWidth() * world.getHeight() / 135500;
    for (int tries = numChasms * 75; numChasms > 0 && tries > 0; --tries) {
        int x = rnd.getInt(50, world.getWidth() - 50);
        int y = rnd.getInt(
            0.5 * world.getUndergroundLevel(),
            world.getUnderworldLevel());
        if (!world.regionPasses(
                x - 5,
                y - 5,
                10,
                10,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            isLocationUsed(x, y, 200, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        --numChasms;
        genChasmAt(
            {x, y},
            rnd.getDouble(9, 20),
            rnd.getDouble(-std::numbers::pi, std::numbers::pi),
            rnd,
            world);
    }

    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        int maxWallClear = std::midpoint(
            world.getSurfaceLevel(x),
            world.getUndergroundLevel());
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.wireRed) {
                tile.wireRed = false;
                if (tile.flag == Flag::border &&
                    rnd.getStableUint(x, y) % 2 == 0) {
                    continue;
                }
                double noise = rnd.getFineNoise(x, y);
                if (hypot(world.spawn, {x, y}) < 60 + 10 * noise) {
                    continue;
                }
                tile.blockID = tile.blockID == TileID::snow ||
                                       tile.blockID == TileID::thinIce
                                   ? TileID::thinIce
                                   : TileID::empty;
                if (y < maxWallClear + 8 * noise) {
                    tile.wallID = WallID::empty;
                }
            }
        }
        if (x != 0) {
            for (int y = 0.5 * world.getUndergroundLevel();
                 y < world.getUnderworldLevel();
                 ++y) {
                Tile &tile = world.getTile(x - 1, y);
                if (tile.blockID == TileID::mud &&
                    world.getBiome(x - 1, y).active == Biome::jungle &&
                    world.isExposed(x - 1, y)) {
                    tile.blockID = TileID::jungleGrass;
                } else if (
                    tile.blockID == TileID::dirt &&
                    y < world.getUndergroundLevel() &&
                    world.getBiome(x - 1, y).active == Biome::forest &&
                    world.isExposed(x - 1, y)) {
                    tile.blockID = TileID::grass;
                }
            }
        }
    });
}
