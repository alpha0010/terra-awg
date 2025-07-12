#ifndef TEMPLE_H
#define TEMPLE_H

#include <functional>
#include <vector>

class World;
class Random;

typedef std::pair<int, int> Point;

void genTemple(Random &rnd, World &world);
Point selectTempleCenter(
    std::function<bool(Point, World &)> isValid,
    Random &rnd,
    World &world);
void clearTempleSurface(Point center, int scanDist, Random &rnd, World &world);
bool canPlaceTempleTreasureAt(int x, int y, World &world);
void addTempleTreasures(
    std::vector<Point> &locations,
    int numRooms,
    Random &rnd,
    World &world);
void addTempleTraps(
    std::vector<Point> &locations,
    int freqCtrl,
    Random &rnd,
    World &world);
void addTempleSpikesAt(Point pos, Random &rnd, World &world);

#endif // TEMPLE_H
