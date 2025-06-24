#include "biomes/hardmode/Hallow.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/Dungeon.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <iostream>

int selectHallowLocation(Random &rnd, World &world)
{
    constexpr auto avoidTiles = frozen::make_set<int>({
        TileID::corruptIce,
        TileID::corruptJungleGrass,
        TileID::crimsand,
        TileID::crimsandstone,
        TileID::crimsonIce,
        TileID::crimsonJungleGrass,
        TileID::crimstone,
        TileID::ebonsand,
        TileID::ebonsandstone,
        TileID::ebonstone,
        TileID::flesh,
        TileID::hardenedCrimsand,
        TileID::hardenedEbonsand,
        TileID::hardenedSand,
        TileID::lesion,
    });
    std::array avoidPoints{
        100,
        world.getWidth() - 100,
        world.getWidth() / 2,
        static_cast<int>(world.desertCenter),
        static_cast<int>(world.jungleCenter),
        static_cast<int>(world.snowCenter),
        world.surfaceEvilCenter,
        computeDungeonCenter(world),
    };
    int scanDist = 0.12 * world.getWidth();
    while (true) {
        int x = rnd.getInt(300, world.getWidth() - 300);
        for (int col : avoidPoints) {
            if (std::abs(col - x) < scanDist / 2) {
                x = -1;
                break;
            }
        }
        if (x != -1) {
            int surface = world.getSurfaceLevel(x);
            if (world.regionPasses(
                    x - scanDist,
                    surface / 2,
                    scanDist,
                    world.getUnderworldLevel() - surface,
                    [&avoidTiles](Tile &tile) {
                        return !avoidTiles.contains(tile.blockID);
                    })) {
                return x;
            }
        }
        --scanDist;
    }
}

void genHallow(Random &rnd, World &world)
{
    std::cout << "Illuminating the world\n";
    int centerX = selectHallowLocation(rnd, world);
    constexpr auto paintedBlocks = frozen::make_set<int>({
        TileID::cloud,
        TileID::rainCloud,
        TileID::snowCloud,
        TileID::jungleGrass,
        TileID::mushroomGrass,
    });
    constexpr auto hallowBlocks = frozen::make_map<int, int>({
        {TileID::grass, TileID::hallowedGrass},
        {TileID::corruptGrass, TileID::hallowedGrass},
        {TileID::crimsonGrass, TileID::hallowedGrass},
        {TileID::stone, TileID::pearlstone},
        {TileID::ebonstone, TileID::pearlstone},
        {TileID::crimstone, TileID::pearlstone},
        {TileID::smoothMarble, TileID::pearlstone},
        {TileID::smoothGranite, TileID::pearlstone},
        {TileID::ice, TileID::hallowedIce},
        {TileID::corruptIce, TileID::hallowedIce},
        {TileID::crimsonIce, TileID::hallowedIce},
        {TileID::sand, TileID::pearlsand},
        {TileID::ebonsand, TileID::pearlsand},
        {TileID::crimsand, TileID::pearlsand},
        {TileID::hardenedSand, TileID::hardenedPearlsand},
        {TileID::hardenedEbonsand, TileID::hardenedPearlsand},
        {TileID::hardenedCrimsand, TileID::hardenedPearlsand},
        {TileID::sandstone, TileID::pearlsandstone},
        {TileID::ebonsandstone, TileID::pearlsandstone},
        {TileID::crimsandstone, TileID::pearlsandstone},
        {TileID::corruptJungleGrass, TileID::jungleGrass},
        {TileID::crimsonJungleGrass, TileID::jungleGrass},
        {TileID::leaf, TileID::aetherium},
        {TileID::mahoganyLeaf, TileID::aetherium},
        {TileID::livingWood, TileID::pearlstone},
        {TileID::livingMahogany, TileID::pearlstone},
        {TileID::slime, TileID::pinkSlime},
        {TileID::lesion, TileID::crystalBlock},
        {TileID::flesh, TileID::crystalBlock},
    });
    std::map<int, int> hallowWalls{
        {WallID::Safe::livingLeaf, WallID::Safe::aetherium},
        {WallID::Unsafe::grass, WallID::Unsafe::hallowedGrass},
        {WallID::Unsafe::flower, WallID::Unsafe::hallowedGrass},
        {WallID::Unsafe::jungle, WallID::Unsafe::hallowedGrass},
        {WallID::Unsafe::corruptGrass, WallID::Unsafe::hallowedGrass},
        {WallID::Unsafe::crimsonGrass, WallID::Unsafe::hallowedGrass},
        {WallID::Unsafe::sandstone, WallID::Unsafe::pearlsandstone},
        {WallID::Unsafe::ebonsandstone, WallID::Unsafe::pearlsandstone},
        {WallID::Unsafe::crimsandstone, WallID::Unsafe::pearlsandstone},
        {WallID::Unsafe::hardenedSand, WallID::Unsafe::hardenedPearlsand},
        {WallID::Unsafe::hardenedEbonsand, WallID::Unsafe::hardenedPearlsand},
        {WallID::Unsafe::hardenedCrimsand, WallID::Unsafe::hardenedPearlsand},
    };
    for (int wallId : WallVariants::dirt) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    for (int wallId : WallVariants::stone) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    for (int wallId : WallVariants::jungle) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    for (int wallId : WallVariants::corruption) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    for (int wallId : WallVariants::crimson) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    for (int wallId :
         {WallID::Unsafe::snow,
          WallID::Unsafe::ice,
          WallID::Unsafe::mushroom,
          WallID::Unsafe::marble,
          WallID::Unsafe::granite,
          WallID::Unsafe::corruptTendril,
          WallID::Unsafe::crimsonBlister}) {
        hallowWalls[wallId] = rnd.select(WallVariants::hallow);
    }
    int scanDist = 0.07 * world.getWidth();
    for (int x = centerX - scanDist; x < centerX + scanDist; ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            auto blockItr = hallowBlocks.find(tile.blockID);
            if (blockItr != hallowBlocks.end()) {
                tile.blockID = blockItr->second;
            }
            auto wallItr = hallowWalls.find(tile.wallID);
            if (wallItr != hallowWalls.end()) {
                tile.wallID = wallItr->second;
            }
            if (paintedBlocks.contains(tile.blockID)) {
                tile.blockPaint = Paint::pink;
            }
        }
    }
}
