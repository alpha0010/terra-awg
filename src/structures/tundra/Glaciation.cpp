#include "structures/tundra/Glaciation.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

void genGlaciation(Random &rnd, World &world)
{
    std::cout << "Glaciation\n";
    rnd.shuffleNoise();
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int lavaLevel = world.getUnderworldLevel() + 0.46 * underworldHeight + 1;
    constexpr auto whiteBlocks = frozen::make_set<int>({
        TileID::ash,
        TileID::ashGrass,
        TileID::clay,
        TileID::corruptGrass,
        TileID::corruptJungleGrass,
        TileID::crimsand,
        TileID::crimsandstone,
        TileID::crimsonGrass,
        TileID::crimsonJungleGrass,
        TileID::crimstone,
        TileID::dirt,
        TileID::ebonsand,
        TileID::ebonsandstone,
        TileID::ebonstone,
        TileID::grass,
        TileID::hallowedGrass,
        TileID::hardenedCrimsand,
        TileID::hardenedEbonsand,
        TileID::hardenedSand,
        TileID::jungleGrass,
        TileID::mud,
        TileID::mushroomGrass,
        TileID::sand,
        TileID::sandstone,
        TileID::slime,
        TileID::stone,
    });
    constexpr auto cyanBlocks = frozen::make_set<int>(
        {TileID::tinOre,
         TileID::leadOre,
         TileID::tungstenOre,
         TileID::platinumOre,
         TileID::palladiumOre,
         TileID::orichalcumOre,
         TileID::titaniumOre});
    constexpr auto skyBlueBlocks = frozen::make_set<int>(
        {TileID::copperOre,
         TileID::ironOre,
         TileID::silverOre,
         TileID::goldOre,
         TileID::cobaltOre,
         TileID::mythrilOre,
         TileID::adamantiteOre,
         TileID::hellstone,
         TileID::hellstoneBrick,
         TileID::obsidianBrick});
    constexpr auto whiteWalls = frozen::make_set<int>(
        {WallID::Safe::hardenedSand,
         WallID::Safe::sandstone,
         WallID::Safe::smoothSandstone,
         WallID::Unsafe::hardenedSand,
         WallID::Unsafe::sandstone});
    constexpr auto skyBlueWalls = frozen::make_set<int>(
        {WallID::Safe::ember,
         WallID::Safe::obsidianBrick,
         WallID::Unsafe::hellstoneBrick,
         WallID::Unsafe::obsidianBrick,
         WallID::Unsafe::ember,
         WallID::Unsafe::cinder,
         WallID::Unsafe::magma,
         WallID::Unsafe::smoulderingStone});
    parallelFor(std::views::iota(0, world.getWidth()), [&](int x) {
        int liquidDepth = 0;
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (!world.conf.unpainted) {
                if (tile.blockPaint == Paint::none) {
                    if (whiteBlocks.contains(tile.blockID)) {
                        tile.blockPaint = rnd.getCoarseNoise(x, y) > 0
                                              ? Paint::white
                                              : Paint::gray;
                    } else if (cyanBlocks.contains(tile.blockID)) {
                        tile.blockPaint = Paint::cyan;
                    } else if (skyBlueBlocks.contains(tile.blockID)) {
                        tile.blockPaint = Paint::skyBlue;
                    }
                }
                if (tile.wallPaint == Paint::none) {
                    if (whiteWalls.contains(tile.wallID)) {
                        tile.wallPaint = rnd.getCoarseNoise(x, y) > 0
                                             ? Paint::white
                                             : Paint::gray;
                    } else if (skyBlueWalls.contains(tile.wallID)) {
                        tile.wallPaint = Paint::skyBlue;
                    }
                }
            }
            double candyCaneNoise = rnd.getFineNoise(x / 3, y / 3);
            if (tile.blockID == TileID::ice) {
                if (candyCaneNoise > 0.37) {
                    tile.blockID = TileID::candyCane;
                } else if (candyCaneNoise < -0.37) {
                    tile.blockID = TileID::greenCandyCane;
                }
            }
            if (tile.wallID == WallID::Unsafe::ice) {
                if (candyCaneNoise > 0.37) {
                    tile.wallID = WallID::Safe::candyCane;
                } else if (candyCaneNoise < -0.37) {
                    tile.wallID = WallID::Safe::greenCandyCane;
                }
            }
            if (tile.blockID != TileID::empty || y > lavaLevel) {
                continue;
            }
            if (tile.liquid != Liquid::none) {
                ++liquidDepth;
                if (liquidDepth > 5 + 2 * rnd.getFineNoise(x, y)) {
                    continue;
                }
            }
            switch (tile.liquid) {
            case Liquid::none:
                liquidDepth = 0;
                break;
            case Liquid::water:
                tile.blockID = TileID::ice;
                tile.liquid = Liquid::none;
                break;
            case Liquid::lava:
                tile.blockID = TileID::thinIce;
                tile.liquid = Liquid::none;
                break;
            case Liquid::honey:
            case Liquid::shimmer: {
                int rndInt = 99999 * (1 + rnd.getFineNoise(x, y));
                if (rndInt % 1523 < 507) {
                    tile.blockID = tile.liquid == Liquid::shimmer
                                       ? TileID::aetherium
                                       : TileID::honey;
                    tile.liquid = Liquid::none;
                } else if (rndInt % 1381 < 690) {
                    tile.blockID = TileID::ice;
                    tile.liquid = Liquid::none;
                }
                break;
            }
            }
        }
    });
}
