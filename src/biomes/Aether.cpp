#include "Aether.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>

void applyAetherDistortion(int centerX, int centerY, double size, World &world)
{
    for (int x = std::max(centerX - size, 0.0); x < centerX + size; ++x) {
        int yMin = std::max(centerY - size, 0.0);
        int yMax = centerY + size;
        double distortion = 23 * std::sin(0.11 * x);
        if (distortion > 0) {
            for (int y = yMin; y < yMax; ++y) {
                int delta = distortion *
                            std::min(
                                1 - std::hypot(x - centerX, y - centerY) / size,
                                0.5);
                if (delta > 0) {
                    world.getTile(x, y) = world.getTile(x, y + delta);
                }
            }
        } else {
            for (int y = yMax; y > yMin; --y) {
                int delta = distortion *
                            std::min(
                                1 - std::hypot(x - centerX, y - centerY) / size,
                                0.5);
                if (delta < 0) {
                    world.getTile(x, y) = world.getTile(x, y + delta);
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
        world.getWidth() * world.getHeight() / rnd.getDouble(160000, 190000);
    applyAetherDistortion(centerX, centerY, size * 3.2, world);
    int maxBubblePos = centerY;
    int maxEditPos = centerY;
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
                tile.blockID = TileID::bubble;
                maxBubblePos = std::max(maxBubblePos, y);
            } else if (noiseVal > 0.09) {
                tile.blockID = TileID::empty;
            } else if (noiseVal > 0.02) {
                tile.blockID = tile.blockID == TileID::empty ? TileID::aetherium
                               : centralPropo < 0.6 ? TileID::heliumMossStone
                                                    : TileID::stone;
            }
            if (noiseVal > 0.019) {
                tile.wallID = WallID::empty;
                maxEditPos = std::max(maxEditPos, y);
            }
        }
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::bubble && world.isExposed(x, y)) {
                tile.echoCoatBlock = true;
            } else if (
                tile.blockID == TileID::heliumMossStone &&
                !world.isExposed(x, y)) {
                tile.blockID = TileID::stone;
            }
        }
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::bubble && !tile.echoCoatBlock) {
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
}
