#ifndef CORRUPTION_H
#define CORRUPTION_H

class World;
class Random;

void genCorruption(Random &rnd, World &world);
void genCorruptionAt(int surfaceX, int undergroundX, Random &rnd, World &world);

#endif // CORRUPTION_H
