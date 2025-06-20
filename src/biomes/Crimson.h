#ifndef CRIMSON_H
#define CRIMSON_H

class World;
class Random;

void genCrimson(Random &rnd, World &world);
void genCrimsonAt(int surfaceX, int undergroundX, Random &rnd, World &world);

#endif // CRIMSON_H
