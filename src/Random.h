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
    std::vector<double> humidity;
    std::vector<double> temperature;
    int noiseWidth;
    int noiseHeight;
    int noiseDeltaX;
    int noiseDeltaY;
    int savedNoiseDeltaX;
    int savedNoiseDeltaY;
    std::map<std::string, int> poolState;
    std::mt19937_64 rnd;

    void computeBlurNoise();

    int getPoolIndex(int size, std::source_location origin);

public:
    Random();

    void setSeed(const std::string &seed);

    std::mt19937_64 &getPRNG()
    {
        return rnd;
    }

    /**
     * Precompute noise samples for other noise functions.
     */
    void initNoise(int width, int height, double scale);
    void initBiomeNoise(
        double scale,
        double humidityOffset,
        double temperatureOffset);
    /**
     * Shift cached noise samples, effectively producing new noise samples,
     * cheaply.
     */
    void shuffleNoise();
    /**
     * Save current noise offsets.
     */
    void saveShuffleState();
    /**
     * Restore saved noise offsets, allowing to query matching an earlier noise
     * state before shuffling.
     */
    void restoreShuffleState();

    /**
     * Random boolean value.
     */
    bool getBool();
    /**
     * Random byte.
     */
    uint8_t getByte();
    /**
     * Random floating point in range.
     */
    double getDouble(double min, double max);
    /**
     * Random integer in range. Range includes both `min` and `max`.
     */
    int getInt(int min, int max);
    /**
     * Get a sample of the blurred noise at the specified location.
     */
    double getBlurNoise(int x, int y) const;
    /**
     * Get a sample of the coarse sized noise at the specified location.
     */
    double getCoarseNoise(int x, int y) const;
    /**
     * Get a sample of the fine detailed noise at the specified location.
     */
    double getFineNoise(int x, int y) const;
    double getHumidity(int x, int y) const;
    double getTemperature(int x, int y) const;
    /**
     * Randomly select integers in a range to split the range into segments.
     */
    std::vector<int> partitionRange(int numSegments, int range);

    /**
     * Select a random value from a list. Repeatedly calling will loop through
     * the list in order, following the first selected item.
     *
     * Repeats are keyed on call site source location; intended use is within a
     * loop/wrapper function.
     */
    template <typename T>
    T pool(
        std::initializer_list<T> list,
        std::source_location origin = std::source_location::current())
    {
        return *(list.begin() + getPoolIndex(list.size(), origin));
    }
    template <typename T>
    auto
    pool(T list, std::source_location origin = std::source_location::current())
    {
        return *(list.begin() + getPoolIndex(list.size(), origin));
    }

    /**
     * Select a random value from a list.
     */
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
