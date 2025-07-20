#include "biomes/hiveQueen/AsteroidField.h"

#include "biomes/AsteroidField.h"
#include "biomes/BiomeUtil.h"

void genAsteroidFieldHiveQueen(Random &rnd, World &world)
{
    doGenAsteroidField(
        [&world](int x, int y, double radius, std::function<void(int, int)> f) {
            int scale = 1.2 * radius;
            Point centroid = getHexCentroid(x, y, scale);
            iterateZone(
                {x, y},
                world,
                [centroid, scale](Point pt) {
                    return getHexCentroid(pt, scale) == centroid;
                },
                [centroid, &f](Point pt) {
                    f(pt.x - centroid.x, pt.y - centroid.y);
                });
        },
        rnd,
        world);
}
