#include "biomes/patches/Cloud.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/Cloud.h"
#include "ids/WallID.h"
#include "vendor/frozen/map.h"

inline constexpr auto tileConversion =
    frozen::make_map<std::pair<Biome, int>, int>({
        {{Biome::snow, TileID::dirt}, TileID::snow},
        {{Biome::snow, TileID::cloud}, TileID::snowCloud},
        {{Biome::desert, TileID::dirt}, TileID::sand},
        {{Biome::jungle, TileID::dirt}, TileID::mud},
        {{Biome::jungle, TileID::cloud}, TileID::rainCloud},
    });

inline constexpr auto wallConversion = frozen::make_map<Biome, int>({
    {Biome::snow, WallID::Unsafe::snow},
    {Biome::desert, WallID::Unsafe::hardenedSand},
    {Biome::jungle, WallID::Unsafe::mud},
});

void genCloudPatches(Random &rnd, World &world)
{
    genCloud(rnd, world);
    parallelFor(std::views::iota(0, world.getWidth()), [&world](int x) {
        int maxY = 0.45 * world.getUndergroundLevel() + 10;
        for (int y = 0; y < maxY; ++y) {
            Tile &tile = world.getTile(x, y);
            Biome biome = world.getBiome(x, y).active;
            auto tileItr = tileConversion.find(std::pair{biome, tile.blockID});
            if (tileItr != tileConversion.end()) {
                tile.blockID = tileItr->second;
            }
            if ((tile.blockID == TileID::dirt || tile.blockID == TileID::mud) &&
                world.isExposed(x, y)) {
                tile.blockID = tile.blockID == TileID::dirt
                                   ? TileID::grass
                                   : TileID::jungleGrass;
            }
            if (tile.wallID == WallID::Unsafe::dirt) {
                auto wallItr = wallConversion.find(biome);
                if (wallItr != wallConversion.end()) {
                    tile.wallID = wallItr->second;
                }
            }
        }
    });
}
