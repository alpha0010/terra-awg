#ifndef RANDOM_H
#define RANDOM_H

#include <map>
#include <random>
#include <source_location>
#include <vector>

class Random
{
private:
    std::vector<double> blurNoise;
    std::vector<double> coarseNoise;
    std::vector<double> fineNoise;
    int noiseWidth;
    int noiseHeight;
    int noiseDeltaX;
    int noiseDeltaY;
    int savedNoiseDeltaX;
    int savedNoiseDeltaY;
    std::map<std::string, int> poolState;
    std::default_random_engine rnd;

    void computeBlurNoise();

    int getPoolIndex(int size, std::source_location origin);

public:
    Random();

    void initNoise(int width, int height, double scale);
    void shuffleNoise();
    void saveShuffleState();
    void restoreShuffleState();

    bool getBool();
    uint8_t getByte();
    double getDouble(double min, double max);
    int getInt(int min, int max);
    double getBlurNoise(int x, int y) const;
    double getCoarseNoise(int x, int y) const;
    double getFineNoise(int x, int y) const;
    std::vector<int> partitionRange(int numSegments, int range);

    template <typename T>
    T pool(
        std::initializer_list<T> list,
        std::source_location origin = std::source_location::current())
    {
        return *(list.begin() + getPoolIndex(list.size(), origin));
    }

    template <typename T> T select(std::initializer_list<T> list)
    {
        return *(list.begin() + getInt(0, list.size() - 1));
    }
    template <typename T> auto select(T list)
    {
        auto first = std::begin(list);
        auto last = std::end(list);
        return *(first + getInt(0, std::distance(first, last) - 1));
    }
};

#endif // RANDOM_H
