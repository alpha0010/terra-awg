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
void applyAetherDistortion(int centerX, int centerY, double size, World &world)
{
    int maxX = std::min<int>(centerX + size, world.getWidth());
    int minY = std::max<int>(centerY - size, 1);
    int maxY = std::min<int>(centerY + size, world.getHeight() - 1);
    for (int x = std::max<int>(centerX - size, 0); x < maxX; ++x) {
        double distortion = 23 * std::sin(0.11 * x);
        if (distortion > 0) {
            for (int y = minY; y < maxY; ++y) {
                int delta = distortion *
                            std::min(
                                1 - std::hypot(x - centerX, y - centerY) / size,
                                0.5);
                if (delta > 0) {
                    world.getTile(x, y) = world.getTile(
                        x,
                        std::min(y + delta, world.getHeight() - 1));
                }
            }
        } else {
            for (int y = maxY - 1; y > minY; --y) {
                int delta = distortion *
                            std::min(
                                1 - std::hypot(x - centerX, y - centerY) / size,
                                0.5);
                if (delta < 0) {
                    world.getTile(x, y) =
                        world.getTile(x, std::max(y + delta, 0));
                }
            }
        }
    }
}

void genAether(Random &rnd, World &world)
{
    std::cout << "Bridging realities\n";
    rnd.shuffleNoise();
    int centerX = world.getWidth() * rnd.getDouble(0.08, 0.30);
    if (rnd.getBool()) {
        centerX = world.getWidth() - centerX;
    }
    int centerY = rnd.getInt(
        (world.getUndergroundLevel() + 2 * world.getCavernLevel()) / 3,
        (world.getCavernLevel() + 5 * world.getUnderworldLevel()) / 6);
    double size =
        world.conf.aetherSize * (25 + world.getWidth() * world.getHeight() /
                                          rnd.getDouble(274000, 384000));
    applyAetherDistortion(centerX, centerY, size * 3.2, world);
    int maxBubblePos = centerY;
    int maxEditPos = centerY;
    std::vector<Point> mossLocations;
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            double centralPropo = std::hypot(x - centerX, y - centerY) / size;
            double noiseVal =
                std::max(
                    std::abs(rnd.getBlurNoise(4 * x, 4 * y)),
                    std::abs(
                        rnd.getBlurNoise(4 * x + centerX, 4 * y + centerY))) *
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
    world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
        for (auto [x, y] : mossLocations) {
            growMossOn(x, y, world);
        }
    });
    fillAetherShimmer(
        centerX,
        centerY,
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
