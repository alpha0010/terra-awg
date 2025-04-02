#ifndef PLANTS_H
#define PLANTS_H

#include <map>
#include <vector>

class World;
class Random;

typedef std::map<int, std::vector<std::pair<int, int>>> LocationBins;

void genPlants(const LocationBins &locations, Random &rnd, World &world);

#endif // PLANTS_H
