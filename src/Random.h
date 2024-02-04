#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <vector>

class Random
{
private:
    std::default_random_engine rnd;

public:
    Random();

    bool getBool();
    uint8_t getByte();
    int getInt(int min, int max);
    std::vector<int> partitionRange(int numSegments, int range);

    template <typename T> T select(const std::vector<T> &list)
    {
        return list[getInt(0, list.size() - 1)];
    }
};

#endif // RANDOM_H
