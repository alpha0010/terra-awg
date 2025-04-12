#include "Cleanup.h"

#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include <iostream>
#include <map>
#include <set>

bool isIsolated(World &world, int x, int y)
{
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            if (world.getTile(x + i, y + j).blockID != TileID::empty) {
                return false;
            }
        }
    }
    return true;
}

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
    // Bit layout
    // 035
    // 1 6
    // 247
    size_t flags = 0;
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            flags <<= 1;
            if (world.getTile(x + i, y + j).blockID == TileID::empty) {
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
        if (world.getTile(x - 2, y).blockID == TileID::empty) {
            return Slope::half;
        }
        [[fallthrough]];
    case 0b00010110:
    case 0b00010111:
        return Slope::topRight;
    case 0b11010100:
    case 0b11110100:
        if (world.getTile(x + 2, y).blockID == TileID::empty) {
            return Slope::half;
        }
        [[fallthrough]];
    case 0b11010000:
    case 0b11110000:
        return Slope::topLeft;
    case 0b00001011:
    case 0b00101011:
    case 0b00001111:
    case 0b00101111:
        return Slope::bottomRight;
    case 0b01101000:
    case 0b11101000:
    case 0b01101001:
    case 0b11101001:
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
        TileID::clay,
        TileID::sand,
        TileID::ash,
        TileID::mud,
        TileID::jungleGrass,
        TileID::mushroomGrass,
        TileID::ebonsand,
        TileID::silt,
        TileID::snow,
        TileID::ice,
        TileID::corruptIce,
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
        TileID::lavaMossStone,
        TileID::sandstone,
        TileID::hardenedSand,
        TileID::hardenedEbonsand,
        TileID::hardenedCrimsand,
        TileID::ebonsandstone,
        TileID::crimsandstone,
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
                if (tile.blockID == TileID::empty || tile.guarded ||
                    !world.isExposed(x, y)) {
                    continue;
                }
                if (tile.wallID != WallID::empty) {
                    // Blend with surrounding walls by matching wall type
                    // used/blank in orthogonal non-block-covered space.
                    std::tie(tile.wallID, tile.wallPaint) =
                        getAttachedOpenWall(world, x, y);
                }
                if (isIsolated(world, x, y)) {
                    tile.blockID = TileID::empty;
                    continue;
                }
                if (world.getTile(x, y + 1).blockID == TileID::empty) {
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
