#ifndef ASTEROIDFIELD_H
#define ASTEROIDFIELD_H

#include <functional>

class World;
class Random;

void genAsteroidField(Random &rnd, World &world);

void doGenAsteroidField(
    std::function<void(int, int, double, std::function<void(int, int)>)>
        iterateAsteroid,
    Random &rnd,
    World &world);

#endif // ASTEROIDFIELD_H
