#include "Cleanup.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

std::pair<int, int> getAttachedOpenWall(World &world, int x, int y)
{
    Tile &origin = world.getTile(x, y);
    std::pair<int, int> res{origin.wallID, origin.wallPaint};
    bool keepOrigin = false;
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID == TileID::empty) {
                if (tile.wallID == WallID::empty) {
                    return {WallID::empty, Paint::none};
                } else if (!keepOrigin) {
                    if (tile.wallID == origin.wallID) {
                        keepOrigin = true;
                    } else {
                        res = {tile.wallID, tile.wallPaint};
                    }
                }
            }
        }
    }
    return keepOrigin ? std::pair{origin.wallID, origin.wallPaint} : res;
}

Slope computeSlope(World &world, int x, int y)
{
    // Bit layout:
    // 035
    // 1 6
    // 247
    size_t flags = 0;
    std::set<int> emptyIds{TileID::empty, TileID::minecartTrack};
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            flags <<= 1;
            if (emptyIds.contains(world.getTile(x + i, y + j).blockID)) {
                flags |= 1;
            }
        }
    }
    switch (flags) {
    case 0b11010110:
    case 0b11110110:
    case 0b11010111:
        return Slope::half;
    case 0b10010110:
    case 0b10010111:
        if (world.getTile(x - 2, y).blockID == TileID::empty &&
            world.getTile(x, y).blockID != TileID::crystalBlock) {
            return Slope::half;
        }
        [[fallthrough]];
    case 0b00010110:
    case 0b00010111:
    case 0b00110110:
        return Slope::topRight;
    case 0b11010100:
    case 0b11110100:
        if (world.getTile(x + 2, y).blockID == TileID::empty &&
            world.getTile(x, y).blockID != TileID::crystalBlock) {
            return Slope::half;
        }
        [[fallthrough]];
    case 0b11010000:
    case 0b11110000:
    case 0b11010001:
        return Slope::topLeft;
    case 0b00001011:
    case 0b00101011:
    case 0b00001111:
    case 0b00101111:
    case 0b10001011:
        return Slope::bottomRight;
    case 0b01101000:
    case 0b11101000:
    case 0b01101001:
    case 0b11101001:
    case 0b01101100:
        return Slope::bottomLeft;
    default:
        return Slope::none;
    }
}

void smoothSurfaces(World &world)
{
    std::cout << "Smoothing surfaces\n";
    std::map<int, int> stablizeBlocks{
        {TileID::sand, TileID::hardenedSand},
        {TileID::ebonsand, TileID::hardenedEbonsand},
        {TileID::pearlsand, TileID::hardenedPearlsand},
        {TileID::silt, TileID::mud},
        {TileID::slush, TileID::snow},
        {TileID::crimsand, TileID::hardenedCrimsand},
        {TileID::shellPile, TileID::hardenedSand}};
    std::set<int> slopedTiles{
        TileID::dirt,
        TileID::stone,
        TileID::grass,
        TileID::corruptGrass,
        TileID::ebonstone,
        TileID::meteorite,
        TileID::clay,
        TileID::sand,
        TileID::obsidian,
        TileID::ash,
        TileID::mud,
        TileID::jungleGrass,
        TileID::mushroomGrass,
        TileID::hallowedGrass,
        TileID::ebonsand,
        TileID::pearlsand,
        TileID::pearlstone,
        TileID::silt,
        TileID::snow,
        TileID::ice,
        TileID::corruptIce,
        TileID::hallowedIce,
        TileID::cloud,
        TileID::livingWood,
        TileID::leaf,
        TileID::slime,
        TileID::flesh,
        TileID::rainCloud,
        TileID::crimsonGrass,
        TileID::crimsonIce,
        TileID::crimstone,
        TileID::slush,
        TileID::hive,
        TileID::honey,
        TileID::crimsand,
        TileID::coralstone,
        TileID::smoothMarble,
        TileID::marble,
        TileID::granite,
        TileID::smoothGranite,
        TileID::pinkSlime,
        TileID::lavaMossStone,
        TileID::livingMahogany,
        TileID::mahoganyLeaf,
        TileID::crystalBlock,
        TileID::sandstone,
        TileID::hardenedSand,
        TileID::hardenedEbonsand,
        TileID::hardenedCrimsand,
        TileID::ebonsandstone,
        TileID::crimsandstone,
        TileID::hardenedPearlsand,
        TileID::pearlsandstone,
        TileID::snowCloud,
        TileID::lesion,
        TileID::kryptonMossStone,
        TileID::xenonMossStone,
        TileID::argonMossStone,
        TileID::neonMossStone,
        TileID::heliumMossStone,
        TileID::ashGrass,
        TileID::corruptJungleGrass,
        TileID::crimsonJungleGrass,
        TileID::aetherium};
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&stablizeBlocks, &slopedTiles, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.guarded || !isSolidBlock(tile.blockID) ||
                    !world.isExposed(x, y)) {
                    continue;
                }
                if (tile.wallID != WallID::empty) {
                    // Blend with surrounding walls by matching wall type
                    // used/blank in orthogonal non-block-covered space.
                    std::tie(tile.wallID, tile.wallPaint) =
                        getAttachedOpenWall(world, x, y);
                }
                if (world.isIsolated(x, y)) {
                    tile.blockID = TileID::empty;
                    continue;
                }
                if (!isSolidBlock(world.getTile(x, y + 1).blockID)) {
                    // Stop falling sand.
                    auto itr = stablizeBlocks.find(tile.blockID);
                    if (itr != stablizeBlocks.end()) {
                        tile.blockID = itr->second;
                    }
                }
                if (slopedTiles.contains(tile.blockID)) {
                    tile.slope = computeSlope(world, x, y);
                }
            }
        });
}

struct MossRegion {
    int blockID;
    int wallID;
    int rndX;
    int rndY;
};

bool convertToMoss(
    int x,
    int y,
    Tile &tile,
    const std::vector<MossRegion> &mosses,
    Random &rnd,
    World &world)
{
    for (const auto &region : mosses) {
        if (rnd.getCoarseNoise(x + region.rndX, y + region.rndY) > 0.6) {
            tile.wallID = region.wallID;
            if (tile.blockID == TileID::stone) {
                tile.blockID = region.blockID;
                growMossOn(x, y, world);
            }
            return true;
        }
    }
    return false;
}

void finalizeWalls(Random &rnd, World &world)
{
    std::cout << "Hardening walls\n";
    std::vector<MossRegion> mosses;
    for (auto [blockID, wallID] :
         {std::pair{TileID::greenMossStone, WallID::Unsafe::greenMossy},
          {TileID::brownMossStone, WallID::Unsafe::brownMossy},
          {TileID::redMossStone, WallID::Unsafe::redMossy},
          {TileID::blueMossStone, WallID::Unsafe::blueMossy},
          {TileID::purpleMossStone, WallID::Unsafe::purpleMossy}}) {
        mosses.emplace_back(
            blockID,
            wallID,
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()));
    }
    std::shuffle(mosses.begin(), mosses.end(), rnd.getPRNG());
    std::map<int, int> stoneWalls;
    for (int wallId : WallVariants::dirt) {
        stoneWalls[wallId] = rnd.select(WallVariants::stone);
    }
    double mossBound =
        (2 * world.getUndergroundLevel() + world.getCavernLevel()) / 3;
    double stoneBound =
        (4 * world.getCavernLevel() + world.getUnderworldLevel()) / 5;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [mossBound, stoneBound, &mosses, &stoneWalls, &rnd, &world](int x) {
            for (int y = world.getUndergroundLevel(); y < world.getHeight();
                 ++y) {
                double threshold = 15 * (mossBound - y) / world.getHeight();
                if (rnd.getCoarseNoise(x, y) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x, y);
                auto itr = stoneWalls.find(tile.wallID);
                if (itr != stoneWalls.end() && world.isExposed(x, y) &&
                    convertToMoss(x, y, tile, mosses, rnd, world)) {
                    continue;
                }
                threshold = 15 * (stoneBound - y) / world.getHeight();
                if (rnd.getCoarseNoise(x, y) < threshold) {
                    continue;
                }
                if (tile.blockID != TileID::dirt && itr != stoneWalls.end()) {
                    tile.wallID = itr->second;
                }
            }
        });
}
