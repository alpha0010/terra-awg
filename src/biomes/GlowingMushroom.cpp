#include "GlowingMushroom.h"

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
        int lastTileID = TileID::empty;
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
                if (lastTileID == TileID::mud) {
                    Tile &prevTile = world.getTile(x, y - 1);
                    if (prevTile.blockID == TileID::mud) {
                        prevTile.blockID = TileID::mushroomGrass;
                    }
                }
                Tile &leftTile = world.getTile(x - 1, y);
                if (leftTile.blockID == TileID::mud) {
                    leftTile.blockID = TileID::mushroomGrass;
                }
            } else {
                switch (tile.blockID) {
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
            lastTileID = tile.blockID;
        }
    }
}

void genGlowingMushroom(Random &rnd, World &world)
{
    std::cout << "Fertilizing glowing mushrooms\n";
    int numFields = std::max(world.getWidth() * world.getHeight() / 3388000, 2);
    int buffer = 0.06 * world.getWidth();
    while (numFields > 0) {
        int centerX = world.getWidth() * rnd.getDouble(0.05, 0.95);
        if (std::abs(world.desertCenter - centerX) < buffer ||
            std::abs(world.jungleCenter - centerX) < buffer ||
            std::abs(world.snowCenter - centerX) < buffer) {
            continue;
        }
        fillMushroomField(
            centerX,
            rnd.getInt(world.getCavernLevel(), world.getUnderworldLevel() - 50),
            rnd.getInt(buffer / 4, buffer / 2),
            rnd,
            world);
        --numFields;
    }
}
