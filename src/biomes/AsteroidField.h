#ifndef ASTEROIDFIELD_H
#define ASTEROIDFIELD_H

#include <functional>

class World;
class Random;

std::pair<int, int>
selectAsteroidFieldLocation(int &width, int height, Random &rnd, World &world);

void genAsteroidField(Random &rnd, World &world);

void doGenAsteroidField(
    std::function<void(int, int, double, std::function<void(int, int)>)>
        iterateAsteroid,
    Random &rnd,
    World &world);

#endif // ASTEROIDFIELD_H
