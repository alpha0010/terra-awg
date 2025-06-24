#include "biomes/hardmode/HmOres.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "vendor/frozen/set.h"
#include <iostream>

bool canSpawnChlorophyte(int x, int y, World &world)
{
    int radius = 8;
    for (int i = -radius; i < radius; ++i) {
        for (int j = -radius; j < radius; ++j) {
            if (std::hypot(i, j) < radius) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::jungleGrass ||
                    tile.blockID == TileID::chlorophyteOre) {
                    return true;
                }
            }
        }
    }
    return false;
}

struct DepositDef {
    int ore;
    int minY;
    int maxY;
    int noiseX;
    int noiseY;
};

void genHardmodeOres(Random &rnd, World &world)
{
    std::cout << "Blessing ore\n";
    rnd.shuffleNoise();
    constexpr auto clearableTiles = frozen::make_set<int>({
        TileID::ash,
        TileID::clay,
        TileID::corruptIce,
        TileID::corruptJungleGrass,
        TileID::crimsand,
        TileID::crimsandstone,
        TileID::crimsonIce,
        TileID::crimsonJungleGrass,
        TileID::crimstone,
        TileID::dirt,
        TileID::ebonsand,
        TileID::ebonsandstone,
        TileID::ebonstone,
        TileID::granite,
        TileID::hardenedCrimsand,
        TileID::hardenedEbonsand,
        TileID::hardenedSand,
        TileID::ice,
        TileID::jungleGrass,
        TileID::marble,
        TileID::mud,
        TileID::mushroomGrass,
        TileID::sand,
        TileID::sandstone,
        TileID::snow,
        TileID::stone,
    });
    std::array<DepositDef, 3> depositNoise({
        {world.cobaltVariant,
         0,
         std::lerp(world.getCavernLevel(), world.getUnderworldLevel(), 0.48),
         rnd.getInt(0, world.getWidth()),
         rnd.getInt(0, world.getHeight())},
        {world.mythrilVariant,
         std::lerp(world.getCavernLevel(), world.getUnderworldLevel(), 0.12),
         std::lerp(world.getCavernLevel(), world.getUnderworldLevel(), 0.73),
         rnd.getInt(0, world.getWidth()),
         rnd.getInt(0, world.getHeight())},
        {world.adamantiteVariant,
         std::lerp(world.getCavernLevel(), world.getUnderworldLevel(), 0.61),
         world.getHeight(),
         rnd.getInt(0, world.getWidth()),
         rnd.getInt(0, world.getHeight())},
    });
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [chlorophyteThreshold = computeOreThreshold(0.7 * world.conf.ore),
         oreThreshold = computeOreThreshold(0.9 * world.conf.ore),
         &clearableTiles,
         &depositNoise,
         &rnd,
         &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (!clearableTiles.contains(tile.blockID) ||
                    (y > world.getUnderworldLevel() &&
                     tile.blockID == TileID::ash)) {
                    continue;
                }
                if (y > world.getUndergroundLevel() &&
                    rnd.getFineNoise(x, y) < chlorophyteThreshold) {
                    if (tile.blockID == TileID::jungleGrass ||
                        (tile.blockID == TileID::mud &&
                         canSpawnChlorophyte(x, y, world))) {
                        tile.blockID = TileID::chlorophyteOre;
                        continue;
                    }
                }
                if (rnd.getCoarseNoise(x, y) < 0.13) {
                    continue;
                }
                for (const auto &row : depositNoise) {
                    if (y > row.minY && y < row.maxY &&
                        rnd.getFineNoise(x + row.noiseX, y + row.noiseY) <
                            oreThreshold) {
                        tile.blockID = row.ore;
                        break;
                    }
                }
            }
        });
}
