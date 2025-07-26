#include "biomes/celebration/AsteroidField.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/AsteroidField.h"
#include "ids/Paint.h"
#include "structures/data/Balloons.h"
#include <iostream>

void genAsteroidFieldCelebration(Random &rnd, World &world)
{
    std::cout << "Inflating asteroids\n";
    int width =
        world.conf.asteroids * rnd.getDouble(0.06, 0.07) * world.getWidth();
    int height = rnd.getDouble(0.18, 0.21) * world.getUndergroundLevel();
    auto [fieldX, fieldY] =
        selectAsteroidFieldLocation(width, height, rnd, world);
    int numAsteroids = width * height / 375;
    for (int tries = 10 * numAsteroids; numAsteroids > 0 && tries > 0;
         --tries) {
        int tileId, paint;
        switch (rnd.getInt(0, 2)) {
        case 0:
            tileId = TileID::sillyPinkBalloon;
            paint = rnd.select(
                {Paint::none, Paint::pink, Paint::red, Paint::violet});
            break;
        case 1:
            tileId = TileID::sillyPurpleBalloon;
            paint = rnd.select(
                {Paint::none, Paint::purple, Paint::blue, Paint::skyBlue});
            break;
        default:
            tileId = TileID::sillyGreenBalloon;
            paint = rnd.select(
                {Paint::none, Paint::green, Paint::lime, Paint::teal});
            break;
        }
        TileBuffer balloon = Data::getBalloon(
            rnd.select(Data::balloons),
            tileId,
            paint,
            world.getFramedTiles());
        int x = rnd.getInt(fieldX, fieldX + width - balloon.getWidth());
        int y = rnd.getInt(fieldY, fieldY + height - balloon.getHeight());
        double centerDist = std::hypot(
            (fieldX + 0.5 * width - x) / width,
            (fieldY + 0.5 * height - y) / height);
        if ((centerDist > 0.48 && fnv1a32pt(x, y) % 11 != 0) ||
            !world.regionPasses(
                x - 1,
                y - 1,
                balloon.getWidth() + 2,
                balloon.getHeight() + 1,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(
                x + balloon.getWidth() / 2 - 1,
                y + balloon.getHeight(),
                3,
                3,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            continue;
        }
        world.placeBuffer(x, y, balloon);
        int ropeLen = rnd.getInt(
            std::midpoint(4, balloon.getWidth()),
            4 + balloon.getHeight());
        for (int j = 0; j < ropeLen; ++j) {
            Tile &tile = world.getTile(
                x + balloon.getWidth() / 2,
                y + balloon.getHeight() + j);
            if (tile.blockID != TileID::empty) {
                break;
            }
            tile.blockID = TileID::rope;
        }
        --numAsteroids;
    }
}
