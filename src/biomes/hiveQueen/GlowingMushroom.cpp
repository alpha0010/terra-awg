#include "biomes/hiveQueen/GlowingMushroom.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

void fillMushroomFieldHex(
    int centerX,
    int fieldFloor,
    Random &rnd,
    World &world)
{
    rnd.shuffleNoise();
    int fieldSize = rnd.getInt(75, 100);
    int centerY = fieldFloor - 0.35 * fieldSize;
    world.mushroomCenter.emplace_back(centerX, centerY);
    int secondaryBlock = rnd.select({TileID::silt, TileID::slime});
    iterateZone(
        {centerX, fieldFloor},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [&](Point pt) {
            auto coarseHexNoise = [&rnd](int x, int y) {
                Point centroid = getHexCentroid(x, y, 10);
                return rnd.getCoarseNoise(centroid.x, centroid.y);
            };
            auto [x, y] = pt;
            Tile &tile = world.getTile(x, y);
            if (std::abs(coarseHexNoise(x, y)) > 0.07 &&
                std::abs(coarseHexNoise(x + centerX, y + centerY)) > 0.07 &&
                y > fieldFloor +
                        5 * rnd.getFineNoise(x, world.getUndergroundLevel()) -
                        0.7 * std::sqrt(std::max(
                                  fieldSize * fieldSize -
                                      std::pow(centerX - x, 2),
                                  0.0)) &&
                y < fieldFloor + 10 * coarseHexNoise(x, 0)) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                for (auto [i, j] : {std::pair{-1, -1}, {-1, 0}, {0, -1}}) {
                    Tile &prevTile = world.getTile(x + i, y + j);
                    if (prevTile.blockID == TileID::mud) {
                        prevTile.blockID = TileID::mushroomGrass;
                    }
                }
            } else {
                switch (tile.blockID) {
                case TileID::grass:
                    tile.blockID = TileID::mushroomGrass;
                    if (tile.wallID != WallID::empty) {
                        tile.wallID = WallID::Unsafe::mushroom;
                    }
                    break;
                case TileID::stone:
                case TileID::clay:
                case TileID::sand:
                case TileID::mud:
                    tile.blockID =
                        world.isExposed(x, y) || rnd.getInt(0, 35) == 0
                            ? TileID::mushroomGrass
                            : TileID::mud;
                    [[fallthrough]];
                case TileID::empty:
                    if (tile.wallID != WallID::empty) {
                        tile.wallID = WallID::Unsafe::mushroom;
                    }
                    break;
                case TileID::dirt:
                    tile.blockID = secondaryBlock;
                    break;
                }
            }
        });
}

void genGlowingMushroomHiveQueen(Random &rnd, World &world)
{
    std::cout << "Fertilizing glowing mushrooms\n";
    int numFields = std::max<int>(
        world.conf.glowingMushroomFreq * world.getWidth() * world.getHeight() /
            3388000,
        2);
    for (int tries = 100 * numFields; numFields > 0 && tries > 0; --tries) {
        int centerX = world.getWidth() * rnd.getDouble(0.05, 0.95);
        int fieldFloor =
            rnd.getInt(world.getCavernLevel(), world.getUnderworldLevel() - 50);
        if (world.getBiome(centerX, fieldFloor).active != Biome::forest ||
            !world.regionPasses(
                centerX - 25,
                fieldFloor - 55,
                50,
                60,
                [](Tile &tile) {
                    return tile.blockID != TileID::marble &&
                           tile.flag != Flag::border;
                })) {
            continue;
        }
        fillMushroomFieldHex(centerX, fieldFloor, rnd, world);
        --numFields;
    }
    int mushroomLevel =
        (world.getCavernLevel() + 3 * world.getUnderworldLevel()) / 4;
    for (int x = 0; x < world.getWidth(); x += 50) {
        for (int y = mushroomLevel; y < world.getUnderworldLevel() + 20;
             y += 50) {
            if (world.getBiome(x, y).active == Biome::jungle &&
                rnd.getInt(0, 5) == 0 &&
                world.regionPasses(x - 8, y - 8, 16, 16, [](Tile &tile) {
                    return tile.wallID != WallID::Unsafe::mushroom;
                })) {
                iterateZone(
                    {x, y},
                    world,
                    [&world](Point pt) {
                        return world.getTile(pt).flag != Flag::border;
                    },
                    [&](Point pt) {
                        Tile &tile = world.getTile(pt);
                        switch (tile.blockID) {
                        case TileID::jungleGrass:
                        case TileID::mud:
                            tile.blockID =
                                tile.flag == Flag::crispyHoney
                                    ? TileID::crispyHoney
                                : tile.blockID == TileID::jungleGrass ||
                                        rnd.getInt(0, 35) == 0
                                    ? TileID::mushroomGrass
                                    : TileID::mud;
                            break;
                        case TileID::silt:
                            tile.blockID = TileID::slime;
                            break;
                        }
                        if (tile.wallID != WallID::empty) {
                            tile.wallID = WallID::Unsafe::mushroom;
                        }
                    });
            }
        }
    }
}
