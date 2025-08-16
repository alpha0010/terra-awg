#include "biomes/doubleTrouble/Corruption.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "biomes/Corruption.h"
#include "vendor/frozen/set.h"
#include <iostream>

void genSecondaryCorruption(Random &rnd, World &world)
{
    std::cout << "Corrupting the world\n";
    rnd.shuffleNoise();
    if (world.conf.dontDigUp) {
        int minX = 0;
        int maxX = world.getWidth();
        if (world.surfaceEvilCenter < world.getWidth() / 2) {
            minX = world.getWidth() / 2;
        } else {
            maxX = world.getWidth() / 2;
        }
        for (auto [surfaceX, undergroundX] : selectEvilLocations(rnd, world)) {
            if (surfaceX < minX || surfaceX > maxX) {
                continue;
            }
            genCorruptionAt(surfaceX, undergroundX, rnd, world);
        }
        return;
    }

    int scanDist = 0.08 * world.getWidth();
    int surfaceX = world.surfaceEvilCenter;
    while (std::abs(surfaceX - world.surfaceEvilCenter) < scanDist ||
           (world.conf.avoidantEvil && !isInBiome(
                                           surfaceX,
                                           world.getUndergroundLevel(),
                                           scanDist / 3,
                                           Biome::forest,
                                           world))) {
        surfaceX = world.getWidth() * rnd.getDouble(0.12, 0.39);
        if (rnd.getBool()) {
            surfaceX = world.getWidth() - surfaceX;
        }
    }
    constexpr auto avoidTiles = frozen::make_set<int>({
        TileID::ebonstone,
        TileID::corruptGrass,
        TileID::corruptJungleGrass,
        TileID::corruptIce,
        TileID::ebonsand,
        TileID::ebonsandstone,
        TileID::hardenedEbonsand,
        TileID::lesion,
        TileID::crimstone,
        TileID::crimsonGrass,
        TileID::crimsonJungleGrass,
        TileID::crimsonIce,
        TileID::crimsand,
        TileID::crimsandstone,
        TileID::hardenedCrimsand,
        TileID::flesh,
    });
    while (true) {
        int undergroundX = world.getWidth() * rnd.getDouble(0.08, 0.92);
        if (world.regionPasses(
                undergroundX - scanDist,
                std::midpoint(
                    world.getCavernLevel(),
                    world.getUnderworldLevel()),
                2 * scanDist,
                1,
                [&avoidTiles](Tile &tile) {
                    return !avoidTiles.contains(tile.blockID);
                })) {
            genCorruptionAt(surfaceX, undergroundX, rnd, world);
            return;
        }
    }
}
