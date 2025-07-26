#include "biomes/hardmode/Hallow.h"

#include "Cleanup.h"
#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/Dungeon.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <iostream>

template <typename Func>
void iterateDiamond(int topHeight, int centerHeight, Func f)
{
    for (int i = 0; i < 2 * topHeight; ++i) {
        for (int j = std::abs(topHeight - i - 0.5); j < topHeight; ++j) {
            f(i, j);
        }
        for (int j = 0; j < centerHeight; ++j) {
            f(i, j + topHeight);
        }
        int maxJ = topHeight - std::abs(topHeight - i - 0.5);
        for (int j = 0; j < maxJ; ++j) {
            f(i, j + topHeight + centerHeight);
        }
    }
}

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
        world.spawn.x,
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

void markForHallow(int centerX, Random &rnd, World &world)
{
    int scanDist = 0.07 * world.getWidth();
    int numDiamonds = rnd.getInt(27, 31);
    for (int iter = 0; iter < numDiamonds; ++iter) {
        int topHeight = 25 + world.getWidth() * world.getHeight() /
                                 rnd.getInt(76800, 230400);
        int centerHeight = rnd.getInt(0.5 * topHeight, 1.5 * topHeight);
        int x =
            rnd.getInt(centerX - scanDist, centerX + scanDist - 2 * topHeight);
        int y = rnd.getInt(
            0.1 * world.getUndergroundLevel(),
            world.getUnderworldLevel());
        iterateDiamond(topHeight, centerHeight, [x, y, &world](int i, int j) {
            world.getTile(x + i, y + j).wireRed = true;
        });
        x += 0.9 * topHeight;
        y += 0.9 * topHeight + 0.45 * centerHeight;
        if (y > world.getSurfaceLevel(x) - 50 && y < world.getHeight() - 100) {
            iterateDiamond(
                topHeight / 10,
                centerHeight / 10,
                [x, y, &world](int i, int j) {
                    world.getTile(x + i, y + j).wireBlue = true;
                });
        }
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
        TileID::hive,
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
    constexpr auto erodeBlocks = frozen::make_set<int>({
        TileID::pearlstone,
        TileID::hallowedIce,
        TileID::pearlsandstone,
    });
    constexpr auto erodeSkipBlocks = frozen::make_set<int>({
        TileID::livingWood,
        TileID::livingMahogany,
    });
    int scanDist = 0.07 * world.getWidth();
    std::vector<std::pair<int, int>> erosion;
    std::vector<std::pair<int, int>> hallowCores;
    markForHallow(centerX, rnd, world);
    for (int x = centerX - scanDist; x < centerX + scanDist; ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (!tile.wireRed) {
                continue;
            }
            tile.wireRed = false;
            if (tile.wireBlue) {
                tile.wireBlue = false;
                tile.blockID = TileID::crystalBlock;
                tile.slope = Slope::none;
                tile.guarded = false;
                hallowCores.emplace_back(x, y);
            }
            auto blockItr = hallowBlocks.find(tile.blockID);
            if (blockItr != hallowBlocks.end()) {
                tile.blockID = blockItr->second;
                if (erodeBlocks.contains(tile.blockID) &&
                    !erodeSkipBlocks.contains(blockItr->first) &&
                    world.isExposed(x, y)) {
                    erosion.emplace_back(x, y);
                }
            }
            auto wallItr = hallowWalls.find(tile.wallID);
            if (wallItr != hallowWalls.end()) {
                tile.wallID = wallItr->second;
            }
            if (!world.conf.unpainted && paintedBlocks.contains(tile.blockID)) {
                tile.blockPaint = tile.blockID == TileID::jungleGrass
                                      ? Paint::cyan
                                      : Paint::pink;
            }
        }
    }
    for (auto [x, y] : erosion) {
        Tile &tile = world.getTile(x, y);
        if (tile.wallID != WallID::empty) {
            std::tie(tile.wallID, tile.wallPaint) =
                getAttachedOpenWall(world, x, y);
        }
    }
    for (auto [x, y] : erosion) {
        world.getTile(x, y).blockID = TileID::empty;
    }
    erosion.clear();
    for (auto [x, y] : hallowCores) {
        if (std::abs(rnd.getFineNoise(2 * x, 2 * y)) > 0.09 &&
            world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                return tile.blockID == TileID::crystalBlock;
            })) {
            erosion.emplace_back(x, y);
        }
    }
    for (auto [x, y] : erosion) {
        Tile &tile = world.getTile(x, y);
        tile.blockID = TileID::pearlstone;
        tile.wallID = WallID::Safe::amethystGemspark;
    }
}
