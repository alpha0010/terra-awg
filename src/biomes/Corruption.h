#ifndef CORRUPTION_H
#define CORRUPTION_H

#include <vector>

class World;
class Random;

std::vector<std::pair<int, int>> selectEvilLocations(Random &rnd, World &world);

void genCorruption(Random &rnd, World &world);
void genCorruptionAt(int surfaceX, int undergroundX, Random &rnd, World &world);

#endif // CORRUPTION_H
