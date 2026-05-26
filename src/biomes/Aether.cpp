#include "Aether.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

/**
 * Distort nearby tiles in a wave pattern.
 */
void applyAetherDistortion(Point center, double size, World &world)
{
    int maxX = std::min<int>(center.x + size, world.getWidth());
    int minY = std::max<int>(center.y - size, 1);
    int maxY = std::min<int>(center.y + size, world.getHeight() - 1);
    for (int x = std::max<int>(center.x - size, 0); x < maxX; ++x) {
        double distortion = 23 * std::sin(0.11 * x);
        if (distortion > 0) {
            for (int y = minY; y < maxY; ++y) {
                int delta = distortion *
                            std::min(1 - hypot(center, {x, y}) / size, 0.5);
                if (delta > 0) {
                    world.getTile(x, y) = world.getTile(
                        x,
                        std::min(y + delta, world.getHeight() - 1));
                }
            }
        } else {
            for (int y = maxY - 1; y > minY; --y) {
                int delta = distortion *
                            std::min(1 - hypot(center, {x, y}) / size, 0.5);
                if (delta < 0) {
                    world.getTile(x, y) =
                        world.getTile(x, std::max(y + delta, 0));
                }
            }
        }
    }
}

std::pair<int, int> applyAetherRift(
    Point center,
    double size,
    std::vector<Point> &mossLocations,
    Random &rnd,
    World &world)
{
    int maxBubblePos = center.y;
    int maxEditPos = center.y;
    for (int x = center.x - size; x < center.x + size; ++x) {
        for (int y = center.y - size; y < center.y + size; ++y) {
            double centralPropo = hypot(center, {x, y}) / size;
            double noiseVal =
                std::max(
                    std::abs(rnd.getBlurNoise(4 * x, 4 * y)),
                    std::abs(
                        rnd.getBlurNoise(4 * x + center.x, 4 * y + center.y))) *
                std::min(1.0, 3 * (1 - centralPropo));
            Tile &tile = world.getTile(x, y);
            if (noiseVal > 0.45) {
                // Solid bubbles.
                tile.blockID = TileID::bubble;
                maxBubblePos = std::max(maxBubblePos, y);
            } else if (noiseVal > 0.09) {
                tile.blockID = TileID::empty;
            } else if (noiseVal > 0.02) {
                // Seal entrances with aetherium.
                tile.blockID = tile.blockID == TileID::empty ? TileID::aetherium
                               : centralPropo < 0.6 ? TileID::heliumMossStone
                                                    : TileID::stone;
                mossLocations.emplace_back(x, y);
            }
            if (noiseVal > 0.019) {
                tile.wallID = WallID::empty;
                maxEditPos = std::max(maxEditPos, y);
            }
        }
    }
    return {maxBubblePos, maxEditPos};
}

std::pair<int, int> applyAetherCrystalline(
    Point center,
    double size,
    std::vector<Point> &mossLocations,
    Random &rnd,
    World &world)
{
    int maxBubblePos = center.y;
    int maxEditPos = center.y;
    for (int x = center.x - size; x < center.x + size; ++x) {
        for (int y = center.y - size; y < center.y + size; ++y) {
            double centerDist = hypot(center, {x, y});
            if (std::abs(centerDist - size) < 1) {
                if (world.getTile(x, y).blockID == TileID::empty) {
                    iterateDiamond(
                        rnd.getInt(3, 5),
                        1,
                        [x, y, &world](int i, int j) {
                            Tile &tile = world.getTile(x + i - 4, y + j - 4);
                            if (tile.blockID == TileID::empty) {
                                tile.blockID = TileID::aetherium;
                            }
                        });
                }
            }
            if (static_cast<int>(999999 * (1 + rnd.getFineNoise(x, y))) % 2131 >
                18) {
                continue;
            }
            int radius = std::min(20 * (1 - centerDist / size), 9.5) *
                         (1 + 0.2 * rnd.getFineNoise(x, y));
            if (radius < 3) {
                continue;
            }
            iterateDiamond(
                radius,
                1,
                [x, y, &maxEditPos, &world](int i, int j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.blockID != TileID::bubble &&
                        tile.blockID != TileID::aetherium) {
                        tile.blockID = TileID::empty;
                    }
                    maxEditPos = std::max(maxEditPos, y + j);
                });
            int offset = rnd.getInt(4, 7);
            if (radius - offset < 3) {
                continue;
            }
            iterateDiamond(
                radius - offset,
                1,
                [x, y, offset, &maxBubblePos, &world](int i, int j) {
                    world.getTile(x + offset + i, y + offset + j).blockID =
                        TileID::bubble;
                    maxBubblePos = std::max(maxBubblePos, y + offset + j);
                });
        }
    }
    auto [skewX, skewY] =
        rnd.select({std::pair{2, 3}, {2, -3}, {3, 2}, {3, -2}});
    int noiseOffset = 10 * size;
    for (int i = -size; i < size; ++i) {
        for (int j = -size; j < size; ++j) {
            double centralPropo = std::hypot(i, j) / size;
            if (centralPropo > 0.95) {
                continue;
            }
            Tile &tile = world.getTile(center.x + i, center.y + j);
            double noiseVal =
                rnd.getFineNoise(noiseOffset + skewX * i + skewY * j, 0);
            if (noiseVal > 0) {
                tile.wallID = WallID::empty;
                if (tile.blockID != TileID::empty &&
                    tile.blockID != TileID::bubble &&
                    tile.blockID != TileID::aetherium) {
                    tile.blockID =
                        centralPropo < 0.6 ? TileID::aetherium : TileID::stone;
                }
            } else if (
                centralPropo < 0.6 && tile.blockID != TileID::empty &&
                tile.blockID != TileID::bubble) {
                tile.blockID = TileID::heliumMossStone;
                mossLocations.emplace_back(center.x + i, center.y + j);
            }
        }
    }
    // Not a trap.
    world.queuedTraps.emplace_back([center, size](Random &, World &world) {
        for (int x = center.x - size; x < center.x + size; ++x) {
            for (int y = center.y - size; y < center.y + size; ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::bubble && tile.echoCoatBlock) {
                    tile.blockID = TileID::crystalBlock;
                    tile.echoCoatBlock = false;
                }
            }
        }
    });
    return {maxBubblePos, maxEditPos};
}

void genAether(Random &rnd, World &world)
{
    std::cout << "Bridging realities\n";
    rnd.shuffleNoise();
    Point center;
    center.x = world.getWidth() * rnd.getDouble(0.08, 0.30);
    if (rnd.getBool()) {
        center.x = world.getWidth() - center.x;
    }
    center.y =
        world.conf.ascent
            ? rnd.getInt(
                  (4 * world.getUndergroundLevel() + world.getCavernLevel()) /
                      5,
                  (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3)
            : rnd.getInt(
                  (world.getUndergroundLevel() + 2 * world.getCavernLevel()) /
                      3,
                  (world.getCavernLevel() + 5 * world.getUnderworldLevel()) /
                      6);
    double size =
        world.conf.aetherSize * (25 + world.getWidth() * world.getHeight() /
                                          rnd.getDouble(274000, 384000));
    applyAetherDistortion(center, size * 3.2, world);
    int maxBubblePos;
    int maxEditPos;
    std::vector<Point> mossLocations;
    switch (world.conf.aether) {
    case AetherBiome::crystalline:
        std::tie(maxBubblePos, maxEditPos) =
            applyAetherCrystalline(center, size, mossLocations, rnd, world);
        break;
    default: // AetherBiome::rift
        std::tie(maxBubblePos, maxEditPos) =
            applyAetherRift(center, size, mossLocations, rnd, world);
        break;
    }
    world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
        for (auto [x, y] : mossLocations) {
            growMossOn(x, y, world);
        }
    });
    fillAetherShimmer(
        center.x,
        center.y,
        size,
        maxBubblePos,
        maxEditPos,
        rnd,
        world);
}

void fillAetherShimmer(
    int centerX,
    int centerY,
    double size,
    int maxBubblePos,
    int maxEditPos,
    Random &rnd,
    World &world)
{
    world.aether = {centerX, centerY};
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::bubble && world.isExposed(x, y)) {
                // Find bubble edges.
                tile.echoCoatBlock = true;
            } else if (
                tile.blockID == TileID::heliumMossStone &&
                !world.isExposed(x, y)) {
                // Remove interior moss tiles.
                tile.blockID = TileID::stone;
            }
        }
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::bubble && !tile.echoCoatBlock) {
                // Replace bubble interiors with shimmer.
                tile.blockID = TileID::empty;
                tile.liquid = Liquid::shimmer;
            }
        }
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = maxBubblePos + 1; y < maxEditPos + 1; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::empty) {
                if (std::hypot(x - centerX, y - centerY) < size - 2) {
                    // Shimmer pool adjacent to the lowest bubble.
                    tile.liquid = Liquid::shimmer;
                } else {
                    Tile &prevTile = world.getTile(x, y - 1);
                    if (prevTile.liquid == Liquid::shimmer) {
                        prevTile.liquid = Liquid::none;
                        prevTile.blockID = TileID::aetherium;
                    }
                }
            }
        }
    }
    embedWaterfalls(
        {centerX - size, centerY - size},
        {centerX + size, centerY + size},
        {TileID::aetherium},
        Liquid::shimmer,
        16,
        rnd,
        world);
}
