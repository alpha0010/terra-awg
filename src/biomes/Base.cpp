#include "Base.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>

void computeSurfaceLevel(Random &rnd, World &world)
{
    double surfaceLevel = rnd.getDouble(
        0.7 * world.getUndergroundLevel(),
        0.8 * world.getUndergroundLevel());
    int center = world.getWidth() / 2;
    int delta = 0;
    int deltaLen = 1;
    int prevY = surfaceLevel;
    // Keep surface terrain mostly level near spawn and oceans.
    for (int x = 0; x < world.getWidth(); ++x) {
        double drop =
            world.conf.sunken
                ? 120 * (1 / (1 + std::exp(0.057 * (180 + center - x))) +
                         1 / (1 + std::exp(0.057 * (180 + x - center)))) -
                      90
                : 0;
        int curY = surfaceLevel + drop +
                   std::min(
                       {0.1 * std::abs(center - x) + 15,
                        0.08 * std::min(x, world.getWidth() - x) + 5,
                        50.0}) *
                       rnd.getCoarseNoise(x, 0);
        world.getSurfaceLevel(x) = curY;
        if (delta == curY - prevY) {
            ++deltaLen;
        } else {
            if (deltaLen > 4 && (delta == 1 || delta == -1)) {
                // Break up boring slopes.
                for (int i = 0; i < deltaLen; ++i) {
                    world.getSurfaceLevel(x - i) +=
                        9 * (0.5 - std::abs(i - 0.5 * deltaLen) / deltaLen) *
                        rnd.getFineNoise(x - 2 * i, 0);
                }
            }
            delta = curY - prevY;
            deltaLen = 1;
        }
        prevY = curY;
    }
}

void scatterResource(Random &rnd, World &world, int resource)
{
    rnd.shuffleNoise();
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [resource, &rnd, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                if (rnd.getFineNoise(x, y) > 0.7) {
                    Tile &tile = world.getTile(x, y);
                    if (tile.blockID != TileID::empty) {
                        tile.blockID = resource;
                    }
                }
            }
        });
}

void genOreVeins(Random &rnd, World &world, int oreRoof, int oreFloor, int ore)
{
    rnd.shuffleNoise();
    double threshold = computeOreThreshold(world.conf.ore);
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [oreRoof, oreFloor, ore, threshold, &rnd, &world](int x) {
            for (int y = oreRoof; y < oreFloor; ++y) {
                if (rnd.getFineNoise(x, y) < threshold) {
                    Tile &tile = world.getTile(x, y);
                    if (tile.blockID != TileID::empty) {
                        tile.blockID = ore;
                    }
                }
            }
        });
}

void genWorldBase(Random &rnd, World &world)
{
    std::cout << "Generating base terrain\n";
    std::vector<std::tuple<int, int, int>> wallVarNoise;
    for (int wallId : WallVariants::dirt) {
        wallVarNoise.emplace_back(
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()),
            wallId);
    }
    computeSurfaceLevel(rnd, world);
    world.spawnY = world.getSurfaceLevel(world.getWidth() / 2) - 1;
    // Fill the world with dirt and stone; mostly dirt near the surface,
    // transitioning to mostly stone deeper down.
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&rnd, &wallVarNoise, &world](int x) {
            // Skip background wall for the first tile in every column.
            bool placeWalls = false;
            for (int y = world.getSurfaceLevel(x); y < world.getHeight(); ++y) {
                double threshold =
                    y < world.getUndergroundLevel()
                        ? 3.0 * y / world.getUndergroundLevel() - 3
                        : static_cast<double>(y - world.getUndergroundLevel()) /
                              (world.getHeight() - world.getUndergroundLevel());
                Tile &tile = world.getTile(x, y);
                tile.blockID = rnd.getFineNoise(x, y) > threshold
                                   ? TileID::dirt
                                   : TileID::stone;
                if (placeWalls) {
                    for (auto [i, j, wallId] : wallVarNoise) {
                        // Patches of dirt wall variants.
                        if (std::abs(rnd.getCoarseNoise(x + i, y + j)) < 0.07) {
                            tile.wallID = wallId;
                            break;
                        }
                    }
                    if (tile.wallID == WallID::empty &&
                        y < world.getUndergroundLevel()) {
                        tile.wallID = tile.blockID == TileID::stone
                                          ? WallID::Unsafe::rockyDirt
                                          : WallID::Unsafe::dirt;
                    }
                } else {
                    placeWalls = true;
                }
            }
        });

    scatterResource(rnd, world, TileID::clay);
    scatterResource(rnd, world, TileID::sand);
    scatterResource(rnd, world, TileID::mud);

    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    std::map<int, int> underworldWalls{{WallID::empty, WallID::empty}};
    for (int wallId : WallVariants::dirt) {
        underworldWalls[wallId] = rnd.select(WallVariants::underworld);
    }
    double hellstoneThreshold = -computeOreThreshold(4.24492 * world.conf.ore);
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [underworldHeight, hellstoneThreshold, &underworldWalls, &rnd, &world](
            int x) {
            int underworldRoof =
                world.getUnderworldLevel() + 0.22 * underworldHeight +
                19 * rnd.getCoarseNoise(x, 0.33 * world.getHeight());
            int underworldFloor =
                world.getUnderworldLevel() + 0.42 * underworldHeight +
                35 * rnd.getCoarseNoise(x, 0.66 * world.getHeight());
            for (int y =
                     world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0);
                 y < world.getHeight();
                 ++y) {
                // Fill underworld with ash, with an empty band across the
                // entire middle.
                Tile &tile = world.getTile(x, y);
                if (y > underworldFloor) {
                    tile.blockID =
                        std::abs(rnd.getFineNoise(x, y)) > hellstoneThreshold
                            ? TileID::hellstone
                            : TileID::ash;
                } else {
                    tile.blockID =
                        y < underworldRoof ? TileID::ash : TileID::empty;
                }
                tile.wallID = underworldWalls[tile.wallID];
            }
        });

    std::cout << "Generating ore veins\n";
    // Add ore deposits in overlapping bands; more valuable ore bands are
    // deeper.
    genOreVeins(
        rnd,
        world,
        0.6 * world.getUndergroundLevel(),
        (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
        world.copperVariant);
    genOreVeins(
        rnd,
        world,
        0.85 * world.getUndergroundLevel(),
        (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
        world.ironVariant);
    genOreVeins(
        rnd,
        world,
        (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
        (world.getCavernLevel() + world.getUnderworldLevel()) / 2,
        world.silverVariant);
    genOreVeins(
        rnd,
        world,
        (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
        world.getUnderworldLevel(),
        world.goldVariant);

    std::cout << "Digging caves\n";
    rnd.shuffleNoise();
    // Save so later generators can match cave structures.
    rnd.saveShuffleState();
    std::mutex ptMtx;
    std::vector<std::pair<int, int>> isolatedPoints;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [underworldHeight, &ptMtx, &isolatedPoints, &rnd, &world](int x) {
            bool nearEdge = x < 350 || x > world.getWidth() - 350;
            int scanState = 0;
            std::vector<std::pair<int, int>> candidates;
            for (int y = 0; y < world.getHeight(); ++y) {
                if (nearEdge && y < 0.9 * world.getUndergroundLevel()) {
                    continue;
                }
                double threshold =
                    y < world.getUndergroundLevel()
                        ? 2.94 - 3.1 * y / world.getUndergroundLevel()
                    : y > world.getUnderworldLevel()
                        ? 3.1 * (y - world.getUnderworldLevel()) /
                                  underworldHeight -
                              0.16
                        : -0.16;
                bool isEmpty = false;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                    rnd.getFineNoise(x, y) > threshold) {
                    // Strings of nearly connected caves, with horizontal bias.
                    Tile &tile = world.getTile(x, y);
                    tile.blockID = TileID::empty;
                    isEmpty = true;
                }
                threshold =
                    y > world.getUnderworldLevel()
                        ? (world.getUnderworldLevel() - y) / 10.0
                        : static_cast<double>(y - world.getUndergroundLevel()) /
                                  (world.getUnderworldLevel() -
                                   world.getUndergroundLevel()) -
                              1;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y)) > 0.55 &&
                    rnd.getFineNoise(x, y) < threshold + 0.1) {
                    // Increasingly large isolated deep caves.
                    Tile &tile = world.getTile(x, y);
                    tile.blockID = TileID::empty;
                    isEmpty = true;
                }
                switch (scanState) {
                case 0:
                    if (isEmpty) {
                        scanState = 1;
                    }
                    break;
                case 1:
                    if (!isEmpty) {
                        scanState = 2;
                    }
                    break;
                case 2:
                    if (isEmpty) {
                        scanState = 1;
                        candidates.emplace_back(x, y - 1);
                    } else {
                        scanState = 0;
                    }
                    break;
                }
            }
            if (!candidates.empty()) {
                std::lock_guard lock{ptMtx};
                isolatedPoints.insert(
                    isolatedPoints.end(),
                    candidates.begin(),
                    candidates.end());
            }
        });
    for (auto [x, y] : isolatedPoints) {
        if (world.isIsolated(x, y)) {
            world.getTile(x, y).blockID = TileID::empty;
            continue;
        }
    }
}
