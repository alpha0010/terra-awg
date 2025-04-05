#ifndef PLANTS_H
#define PLANTS_H

#include <map>
#include <vector>

class World;
class Random;

typedef std::pair<int, int> Point;
typedef std::map<int, std::vector<Point>> LocationBins;

void genPlants(const LocationBins &locations, Random &rnd, World &world);

#endif // PLANTS_H
