#ifndef TRAPS_H
#define TRAPS_H

class World;
class Random;

void addChestTraps(int x, int y, Random &rnd, World &world);
void genTraps(Random &rnd, World &world);

#endif // TRAPS_H
