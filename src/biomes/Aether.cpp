#include "Aether.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
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
            if (rnd.getStableUint(x, y) % 2131 > 18) {
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

void doAetherFoliage(int x, int y, Random &rnd, World &world)
{
    double radius = rnd.getDouble(4, 8);
    for (int i = -radius; i < radius; ++i) {
        for (int j = -radius; j < radius; ++j) {
            if (std::hypot(i, j) / radius <
                0.6 + 0.6 * rnd.getFineNoise(x + i, y + j)) {
                Tile &tile = world.getTile(x + i, y + j);
                tile.blockID = TileID::mahoganyLeaf;
                tile.wallID = WallID::Safe::aetherium;
            }
        }
    }
}

std::pair<int, int> applyAetherGrove(
    Point center,
    double size,
    std::vector<Point> &mossLocations,
    Random &rnd,
    World &world)
{
    int maxEditPos = center.y;
    int maxBubblePos = center.y + 0.5 * size;
    for (int i = -size; i < size; ++i) {
        for (int j = -size; j < size; ++j) {
            double centralPropo = std::hypot(i, j) / size;
            double noiseVal = 2 * centralPropo - 0.32 -
                              std::abs(rnd.getCoarseNoise(
                                  2 * (center.x + i + size),
                                  2 * (center.y + j + size)));
            Tile &tile = world.getTile(center.x + i, center.y + j);
            if (noiseVal < 0.87) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                maxEditPos = std::max(maxEditPos, center.y + j);
                double threshold = 0.04 - 0.1 * j / size;
                if (std::abs(rnd.getBlurNoise(
                        5 * (center.x + i + size),
                        3 * (center.y + j + size))) < threshold) {
                    tile.blockID =
                        rnd.getFineNoise(center.x + i, center.y + j) > 0
                            ? TileID::heliumMossStone
                            : TileID::aetherium;
                    mossLocations.emplace_back(center.x + i, center.y + j);
                }
            } else if (std::midpoint(noiseVal, centralPropo - 0.1) < 1) {
                tile.blockID = tile.blockID == TileID::empty ? TileID::aetherium
                                                             : TileID::stone;
            } else if (
                centralPropo < 1 && center.y + j > maxBubblePos &&
                tile.blockID == TileID::empty) {
                tile.guarded = true;
            }
        }
    }
    maxBubblePos = (2 * maxEditPos + center.y) / 3;

    double trunkRadius = std::max(2.5, size / 22);
    int leafCenter = (maxEditPos + 4 * center.y) / 5;
    for (int y = leafCenter; y <= maxEditPos; ++y) {
        int iMin = 2.5 * rnd.getFineNoise(center.x, y) - trunkRadius;
        int iMax = 2.5 * rnd.getFineNoise(center.x + 100, y) + trunkRadius;
        for (int i = iMin; i <= iMax; ++i) {
            Tile &tile = world.getTile(center.x + i, y);
            if (tile.blockID != TileID::empty) {
                continue;
            }
            if (i > iMin && i < iMax) {
                tile.wallID = WallID::Safe::feywood;
            }
            if (rnd.getFineNoise(4 * (center.x + i), 4 * y) < 0.4) {
                tile.blockID = TileID::livingMahogany;
            }
        }
    }
    double scaleX = 3;
    double scaleY = 5.3;
    for (int i = -size / scaleX; i < size / scaleX; ++i) {
        for (int j = -size / scaleY; j < size / scaleY; ++j) {
            double centralPropo = std::hypot(i * scaleX, j * scaleY) / size;
            if (centralPropo > 1) {
                continue;
            }
            uint32_t threshold = 7 * (1 - centralPropo) + 4.5;
            if (rnd.getStableUint(center.x + i, leafCenter + j) % 107 <
                threshold) {
                doAetherFoliage(center.x + i, leafCenter + j, rnd, world);
            }
            if (centralPropo < 0.9 && std::abs(rnd.getFineNoise(
                                          2 * (center.x + i),
                                          2 * (leafCenter + j))) > 0.1) {
                Tile &tile = world.getTile(center.x + i, leafCenter + j);
                if (tile.wallID == WallID::empty) {
                    tile.wallID = WallID::Safe::aetherium;
                }
            }
        }
    }

    // Not a trap.
    world.getTile(center).flag = Flag::anchor;
    world.queuedTraps.emplace_back([anchor = center,
                                    size,
                                    width = 6 + size / scaleX,
                                    minY = leafCenter - 6 - size / scaleY,
                                    maxY = maxEditPos +
                                           1](Random &rnd, World &world) {
        Point center = findNearestAnchor(anchor, world);
        world.getTile(center).flag = Flag::none;
        for (int i = -width; i < width; ++i) {
            for (int y = minY; y < maxY; ++y) {
                Tile &tile = world.getTile(center.x + i, y);
                if (tile.liquid != Liquid::shimmer) {
                    tile.liquid = Liquid::none;
                }
                if (tile.blockID == TileID::mahoganyLeaf) {
                    switch (rnd.getStableUint(center.x + i, y) % 31) {
                    case 0:
                        tile.blockID = TileID::aetherium;
                        break;
                    case 1:
                        if (!world.isExposed(center.x + i, y)) {
                            tile.blockID = TileID::sillyPinkBalloon;
                            break;
                        }
                        [[fallthrough]];
                    default:
                        tile.blockPaint = Paint::negative;
                        if (rnd.getFineNoise(center.x + 3 * i, 4 * y) > 0.45) {
                            tile.actuated = true;
                        }
                        break;
                    }
                } else if (tile.blockID == TileID::livingMahogany) {
                    tile.blockPaint = Paint::brown;
                }
            }
        }
        for (int i = -size; i < size; ++i) {
            for (int j = -size; j < 0; ++j) {
                if (std::hypot(i, j) < size) {
                    Tile &tile = world.getTile(center.x + i, center.y + j);
                    if (tile.liquid == Liquid::water ||
                        tile.liquid == Liquid::lava) {
                        tile.liquid = Liquid::shimmer;
                    }
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
    case AetherBiome::grove:
        std::tie(maxBubblePos, maxEditPos) =
            applyAetherGrove(center, size, mossLocations, rnd, world);
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
            if (y > maxEditPos) {
                tile.guarded = false;
            }
        }
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = maxBubblePos + 1; y < maxEditPos + 1; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.guarded) {
                tile.guarded = false;
                continue;
            }
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
