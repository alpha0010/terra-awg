#include "Forest.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>

typedef std::pair<double, double> Pointf;

void drawRect(
    Pointf topLeft,
    Pointf bottomRight,
    double skewX,
    double skewY,
    World &world)
{
    int width = bottomRight.first - topLeft.first;
    int height = bottomRight.second - topLeft.second;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            Tile &tile = world.getTile(
                topLeft.first + i + skewX * j,
                topLeft.second + j + skewY * i);
            tile.blockID = TileID::livingWood;
            tile.wallID = WallID::Unsafe::livingWood;
        }
    }
}

void drawLine(Pointf from, Pointf to, double width, World &world)
{
    double deltaX = std::abs(to.first - from.first);
    double deltaY = std::abs(to.second - from.second);
    if (deltaY > deltaX) {
        if (from.second > to.second) {
            std::swap(from, to);
        }
        drawRect(
            {std::floor(from.first - width), std::floor(from.second)},
            {std::ceil(from.first + width), std::ceil(to.second)},
            (to.first - from.first) / deltaY,
            0,
            world);
    } else {
        if (from.first > to.first) {
            std::swap(from, to);
        }
        drawRect(
            {std::floor(from.first), std::floor(from.second - width)},
            {std::ceil(to.first), std::ceil(from.second + width)},
            0,
            (to.second - from.second) / deltaX,
            world);
    }
}

void growLeaves(Pointf from, Pointf to, double leafSpan, World &world)
{
    int minX = std::floor(std::min(from.first, to.first) - leafSpan);
    int maxX = std::ceil(std::max(from.first, to.first) + leafSpan);
    int minY = std::floor(std::min(from.second, to.second) - leafSpan);
    int maxY = std::ceil(std::max(from.second, to.second) + leafSpan);
    for (int x = minX; x < maxX; ++x) {
        for (int y = minY; y < maxY; ++y) {
            if (std::hypot(x - from.first, y - from.second) +
                    std::hypot(x - to.first, y - to.second) <
                2 * leafSpan) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::empty) {
                    tile.blockID = TileID::leaf;
                    tile.wallID = WallID::Safe::livingLeaf;
                }
            }
        }
    }
}

void growBranch(
    Pointf from,
    double weight,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 0.5) {
        return;
    }
    angle = (3 * angle - std::numbers::pi / 2) / 4;
    Pointf to{
        from.first + 2.1 * weight * std::cos(angle),
        from.second + 2.1 * weight * std::sin(angle)};
    drawLine(from, to, weight / 2, world);
    if (weight < 2.6) {
        growLeaves(from, to, std::max(5.5, 4 * weight), world);
    }
    double threshold = rnd.getDouble(0, 1);
    if (threshold < 0.45) {
        growBranch(
            to,
            rnd.getDouble(0.75, 0.88) * weight,
            angle + rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8),
            rnd,
            world);
        if (threshold < 0.32) {
            growBranch(
                to,
                rnd.getDouble(0.3, 0.5) * weight,
                angle + (threshold < 0.16 ? std::numbers::pi / 2
                                          : -std::numbers::pi / 2),
                rnd,
                world);
        }
    } else {
        double branchDistr = rnd.getDouble(0.55, 0.75);
        growBranch(
            to,
            branchDistr * weight,
            angle + rnd.getDouble(-std::numbers::pi / 2, -std::numbers::pi / 4),
            rnd,
            world);
        growBranch(
            to,
            (1.3 - branchDistr) * weight,
            angle + rnd.getDouble(std::numbers::pi / 4, std::numbers::pi / 2),
            rnd,
            world);
    }
}

void growRoot(
    Pointf from,
    double weight,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 0.5) {
        return;
    }
    angle = (4 * angle + std::numbers::pi / 2) / 5;
    Pointf to{
        from.first + 1.8 * weight * std::cos(angle),
        from.second + 1.8 * weight * std::sin(angle)};
    drawLine(from, to, weight / 2, world);
    if (rnd.getDouble(0, 1) < 0.6) {
        growRoot(
            to,
            rnd.getDouble(0.9, 0.99) * weight,
            angle + rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8),
            rnd,
            world);
    } else {
        double rootDistr = rnd.getDouble(0.5, 0.8);
        growRoot(
            to,
            rootDistr * weight,
            angle + rnd.getDouble(-std::numbers::pi / 2, -std::numbers::pi / 4),
            rnd,
            world);
        growRoot(
            to,
            (1.3 - rootDistr) * weight,
            angle + rnd.getDouble(std::numbers::pi / 4, std::numbers::pi / 2),
            rnd,
            world);
    }
}

void growLivingTree(double x, double y, Random &rnd, World &world)
{
    double weight = rnd.getDouble(5, 10);
    growBranch(
        {x, y},
        weight,
        rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8) -
            std::numbers::pi / 2,
        rnd,
        world);
    growRoot(
        {x - 0.18 * weight, y},
        0.58 * weight,
        rnd.getDouble(std::numbers::pi / 8, std::numbers::pi / 4) +
            std::numbers::pi / 2,
        rnd,
        world);
    growRoot(
        {x + 0.18 * weight, y},
        0.58 * weight,
        rnd.getDouble(-std::numbers::pi / 4, -std::numbers::pi / 8) +
            std::numbers::pi / 2,
        rnd,
        world);
}

void growLivingTrees(Random &rnd, World &world)
{
    auto partitions =
        rnd.partitionRange(world.getWidth() / 1280, world.getWidth());
    for (int partition : partitions) {
        int numTrees = rnd.getInt(3, 6);
        for (int x = partition - 25 * numTrees; numTrees > 0;
             x += rnd.getInt(45, 55), --numTrees) {
            if (std::abs(x - world.getWidth() / 2) < 20) {
                continue;
            }
            int y = world.spawnY;
            while (world.getTile(x, y - 1).blockID != TileID::empty && y > 0) {
                --y;
            }
            while (world.getTile(x, y).blockID == TileID::empty &&
                   y < world.getUndergroundLevel()) {
                ++y;
            }
            if (world.getTile(x, y).blockID == TileID::grass) {
                growLivingTree(x, y, rnd, world);
            }
        }
    }
}

void genForest(Random &rnd, World &world)
{
    std::cout << "Nurturing forests\n";
    rnd.shuffleNoise();
    // Grow grass.
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getUndergroundLevel(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::dirt) {
                if (world.isExposed(x, y) ||
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            100 ==
                        0) {
                    tile.blockID = TileID::grass;
                }
            } else if (
                tile.blockID == TileID::empty &&
                tile.wallID == WallID::Unsafe::dirt) {
                tile.wallID = rnd.getFineNoise(x, y) > 0
                                  ? WallID::Unsafe::grass
                                  : WallID::Unsafe::flower;
            }
        }
    }
    // Add living tree clumps.
    growLivingTrees(rnd, world);
}
