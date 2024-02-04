#include "Random.h"

Random::Random() : rnd(std::random_device{}()) {}

bool Random::getBool()
{
    return getInt(0, 1) == 0;
}

uint8_t Random::getByte()
{
    return getInt(0, 0xff);
}

int Random::getInt(int min, int max)
{
    std::uniform_int_distribution<> dist(min, max);
    return dist(rnd);
}

std::vector<int> Random::partitionRange(int numSegments, int range)
{
    std::uniform_real_distribution<> dist(0.1, 1.0);
    double total = 0;
    std::vector<double> segments;
    for (int i = 0; i < numSegments; ++i) {
        total += dist(rnd);
        segments.push_back(total);
    }
    segments.pop_back();
    std::vector<int> partitions;
    for (double seg : segments) {
        partitions.push_back(range * (seg / total));
    }
    return partitions;
}
