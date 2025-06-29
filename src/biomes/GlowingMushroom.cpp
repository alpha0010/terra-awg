#include "GlowingMushroom.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>

void fillMushroomField(
    int centerX,
    int fieldFloor,
    int fieldSize,
    Random &rnd,
    World &world)
{
    rnd.shuffleNoise();
    int centerY = fieldFloor - 0.35 * fieldSize;
    world.mushroomCenter.emplace_back(centerX, centerY);
    int secondaryBlock = rnd.select({TileID::silt, TileID::slime});
    for (int x = centerX - 1.4 * fieldSize; x < centerX + 1.4 * fieldSize;
         ++x) {
        for (int y = fieldFloor - fieldSize; y < fieldFloor + fieldSize / 2;
             ++y) {
            double threshold =
                4 * std::hypot(x - centerX, 2.1 * (y - centerY)) / fieldSize -
                5;
            if (rnd.getFineNoise(x, y) < threshold) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            if (std::abs(rnd.getCoarseNoise(x, y)) > 0.07 &&
                std::abs(rnd.getCoarseNoise(x + centerX, y + centerY)) > 0.07 &&
                y > fieldFloor +
                        5 * rnd.getFineNoise(x, world.getUndergroundLevel()) -
                        0.7 * std::sqrt(std::max(
                                  fieldSize * fieldSize -
                                      std::pow(centerX - x, 2),
                                  0.0)) &&
                y < fieldFloor + 10 * rnd.getCoarseNoise(x, 0)) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                for (auto [i, j] : {std::pair{-1, -1}, {-1, 0}, {0, -1}}) {
                    Tile &prevTile = world.getTile(x + i, y + j);
                    if (prevTile.blockID == TileID::mud ||
                        (prevTile.blockID == TileID::marble &&
                         fnv1a32pt(x + i, y + j) % 11 > 4)) {
                        prevTile.blockID = TileID::mushroomGrass;
                    }
                }
            } else {
                switch (tile.blockID) {
                case TileID::marble:
                    if (world.isExposed(x, y) && fnv1a32pt(x, y) % 11 > 4) {
                        tile.blockID = TileID::mushroomGrass;
                        tile.wallID = WallID::Unsafe::mushroom;
                    }
                    break;
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
                case TileID::jungleGrass:
                case TileID::ice:
                case TileID::sandstone:
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
                default:
                    break;
                }
            }
        }
    }
}

void genGlowingMushroom(Random &rnd, World &world)
{
    std::cout << "Fertilizing glowing mushrooms\n";
    int numFields = std::max<int>(
        world.conf.glowingMushroomFreq * world.getWidth() * world.getHeight() /
            3388000,
        2);
    int maxTries = numFields * 2500;
    for (int tries = 0; numFields > 0 && tries < maxTries; ++tries) {
        int buffer = (world.conf.glowingMushroomSize > 1.0
                          ? std::lerp(
                                world.conf.glowingMushroomSize,
                                1.0,
                                static_cast<double>(tries) / maxTries)
                          : world.conf.glowingMushroomSize) *
                     0.06 * world.getWidth();
        int centerX = world.getWidth() * rnd.getDouble(0.05, 0.95);
        int fieldFloor =
            rnd.getInt(world.getCavernLevel(), world.getUnderworldLevel() - 50);
        if (world.conf.patches) {
            if (world.getBiome(centerX, fieldFloor).forest < 0.99) {
                continue;
            }
        } else if (
            std::abs(world.desertCenter - centerX) < buffer ||
            std::abs(world.jungleCenter - centerX) < buffer ||
            std::abs(world.snowCenter - centerX) < buffer) {
            continue;
        }
        fillMushroomField(
            centerX,
            fieldFloor,
            rnd.getInt(buffer / 4, buffer / 2),
            rnd,
            world);
        --numFields;
    }
}
