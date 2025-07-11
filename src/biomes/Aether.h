#ifndef AETHER_H
#define AETHER_H

class World;
class Random;

void genAether(Random &rnd, World &world);
void fillAetherShimmer(
    int centerX,
    int centerY,
    double size,
    int maxBubblePos,
    int maxEditPos,
    Random &rnd,
    World &world);

#endif // AETHER_H
