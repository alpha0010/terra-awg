#ifndef RANDOM_H
#define RANDOM_H

#include <random>
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
    std::default_random_engine rnd;

public:
    Random();

    void initNoise(int width, int height, double scale);
    void computeBlurNoise();
    void shuffleNoise();

    bool getBool();
    uint8_t getByte();
    double getDouble(double min, double max);
    int getInt(int min, int max);
    double getBlurNoise(int x, int y) const;
    double getCoarseNoise(int x, int y) const;
    double getFineNoise(int x, int y) const;
    std::vector<int> partitionRange(int numSegments, int range);

    template <typename It> auto select(It first, It last) -> decltype(*first)
    {
        return *(first + getInt(0, std::distance(first, last) - 1));
    }
    template <typename T> T select(std::initializer_list<T> list)
    {
        return *(list.begin() + getInt(0, list.size() - 1));
    }
};

#endif // RANDOM_H
