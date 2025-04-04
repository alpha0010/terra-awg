#include "GlowingMoss.h"

#include "Random.h"
#include "World.h"
#include <iostream>
#include <set>

bool isSolidArea(int x, int y, int size, World &world)
{
    std::set<int> allowedTiles{TileID::dirt, TileID::stone, TileID::clay};
    return world.regionPasses(x, y, size, size, [&allowedTiles](Tile &tile) {
        return allowedTiles.contains(tile.blockID);
    });
}

std::pair<int, int> findSolidArea(int minY, int maxY, Random &rnd, World &world)
{
    int size = 8;
    while (true) {
        int x = rnd.getInt(0, world.getWidth() - size);
        int y = rnd.getInt(minY, maxY - size);
        if (isSolidArea(x, y, size, world)) {
            return {x + size / 2, y + size / 2};
        }
    }
}

void fillGlowingMossCave(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    auto [centerX, centerY] = findSolidArea(
        world.getCavernLevel(),
        world.getUnderworldLevel(),
        rnd,
        world);
    double size =
        world.getWidth() * world.getHeight() / rnd.getDouble(95000, 210000);
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::dirt || tile.blockID == TileID::stone) {
                double threshold = std::hypot(x - centerX, y - centerY) / size;
                if (std::max(
                        std::abs(rnd.getBlurNoise(5 * x, 5 * y)),
                        std::abs(rnd.getBlurNoise(7 * x, 7 * y))) >
                    std::max(threshold, 0.53)) {
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
    int mossType = rnd.select(
        {TileID::kryptonMossStone,
         TileID::xenonMossStone,
         TileID::argonMossStone,
         TileID::neonMossStone});
    if (centerY >
        (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5) {
        mossType = rnd.select({mossType, (int)TileID::lavaMossStone});
    }
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            double threshold =
                4 * std::hypot(x - centerX, y - centerY) / size - 3;
            if (rnd.getFineNoise(x, y) > threshold) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::stone && world.isExposed(x, y)) {
                    tile.blockID = mossType;
                }
            }
        }
    }
}

void genGlowingMoss(Random &rnd, World &world)
{
    std::cout << "Energizing moss\n";
    int numCaves = rnd.getInt(2, 8);
    for (int i = 0; i < numCaves; ++i) {
        fillGlowingMossCave(rnd, world);
    }
}
