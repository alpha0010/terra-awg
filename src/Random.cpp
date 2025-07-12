#include "Random.h"

#include "Util.h"
#include "vendor/OpenSimplexNoise.hpp"
#include <algorithm>
#include <iostream>
#include <numbers>

Random::Random() : noiseWidth(0), noiseHeight(0), noiseDeltaX(0), noiseDeltaY(0)
{
    std::string tmpSeed = std::to_string(std::random_device{}());
    tmpSeed += '-';
    auto now = std::chrono::high_resolution_clock::now();
    tmpSeed +=
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
                           now.time_since_epoch())
                           .count());
    setSeed(tmpSeed);
}

void Random::setSeed(const std::string &seed)
{
    uint64_t hash = 14695981039346656037u;
    for (auto c : seed) {
        hash ^= c;
        hash *= 1099511628211;
    }
    rnd.seed(hash);
}

void Random::initNoise(int width, int height, double scale)
{
    std::cout << "Sampling noise\n";
    fineNoise.resize(width * height);
    coarseNoise.resize(width * height);
    std::uniform_int_distribution<int64_t> dist(
        0,
        std::numeric_limits<int64_t>::max());
    OpenSimplexNoise noise{dist(rnd)};
    double radiusX = scale * width * 0.5 * std::numbers::inv_pi;
    double radiusY = scale * height * 0.5 * std::numbers::inv_pi;
    parallelFor(
        std::views::iota(0, width),
        [width, height, radiusX, radiusY, &noise, this](int x) {
            double tX = 2 * std::numbers::pi * x / width;
            double x1 = radiusX * std::cos(tX);
            double x2 = radiusX * std::sin(tX);
            for (int y = 0; y < height; ++y) {
                double tY = 2 * std::numbers::pi * y / height;
                double y1 = radiusY * std::cos(tY);
                double y2 = radiusY * std::sin(tY);
                // Seamless looping 2d noise with fractal details.
                fineNoise[x * height + y] =
                    noise.Evaluate(x1, x2, y1, y2) +
                    0.5 * noise.Evaluate(2 * x1, 2 * x2, 2 * y1, 2 * y2) +
                    0.25 * noise.Evaluate(4 * x1, 4 * x2, 4 * y1, 4 * y2);
                coarseNoise[x * height + y] =
                    noise.Evaluate(x1 / 8, x2 / 8, y1 / 8, y2 / 8) +
                    0.5 * noise.Evaluate(x1 / 4, x2 / 4, y1 / 4, y2 / 4) +
                    0.25 * noise.Evaluate(x1 / 2, x2 / 2, y1 / 2, y2 / 2) +
                    0.125 * fineNoise[x * height + y];
            }
        });

    noiseWidth = width;
    noiseHeight = height;

    computeBlurNoise();
}

void Random::computeBlurNoise()
{
    std::cout << "Blurring noise\n";
    blurNoise.resize(coarseNoise.size());
    // Fast approximate Gaussian blur via horizontal/vertical smearing with
    // rolling averages.
    parallelFor(std::views::iota(0, noiseWidth), [this](int x) {
        double accu = 0;
        for (int y = noiseHeight - 40; y < noiseHeight; ++y) {
            accu = 0.9 * accu + 0.1 * coarseNoise[x * noiseHeight + y];
        }
        for (int y = 0; y < noiseHeight; ++y) {
            accu = 0.9 * accu + 0.1 * coarseNoise[x * noiseHeight + y];
            blurNoise[x * noiseHeight + y] = accu;
        }
    });
    parallelFor(std::views::iota(0, noiseHeight), [this](int y) {
        double accu = 0;
        for (int x = noiseWidth - 40; x < noiseWidth; ++x) {
            accu = 0.9 * accu + 0.1 * blurNoise[x * noiseHeight + y];
        }
        for (int x = 0; x < noiseWidth; ++x) {
            accu = 0.9 * accu + 0.1 * blurNoise[x * noiseHeight + y];
            blurNoise[x * noiseHeight + y] = accu;
        }
    });
}

void Random::initBiomeNoise(
    double scale,
    double humidityOffset,
    double temperatureOffset,
    bool hiveQueen)
{
    std::cout << "Measuring weather\n";
    humidity.resize(noiseWidth * noiseHeight);
    temperature.resize(noiseWidth * noiseHeight);
    std::uniform_int_distribution<int64_t> dist(
        0,
        std::numeric_limits<int64_t>::max());
    OpenSimplexNoise noise{dist(rnd)};
    parallelFor(
        std::views::iota(0, noiseWidth),
        [scale, humidityOffset, temperatureOffset, hiveQueen, &noise, this](
            int x) {
            double offset = scale * (noiseWidth + noiseHeight);
            double xS = 1.4 * scale * x;
            for (int y = 0; y < noiseHeight; ++y) {
                double yS = scale * y;
                int index = x * noiseHeight + y;
                humidity[index] = noise.Evaluate(xS, yS) +
                                  0.5 * noise.Evaluate(2 * xS, 2 * yS) +
                                  0.25 * noise.Evaluate(4 * xS, 4 * yS) +
                                  humidityOffset;
                temperature[index] =
                    noise.Evaluate(offset + xS, offset + yS) +
                    0.5 * noise.Evaluate(offset + 2 * xS, offset + 2 * yS) +
                    0.25 * noise.Evaluate(offset + 4 * xS, offset + 4 * yS) +
                    std::max(0.01 * (y + 355 - noiseHeight), 0.0) +
                    temperatureOffset;
                if (hiveQueen) {
                    double jungleBoost = std::clamp(
                        0.7 - 2.4 * std::abs(x - 0.5 * noiseWidth) / noiseWidth,
                        0.0,
                        0.5);
                    humidity[index] =
                        std::lerp(humidity[index], 0.82, jungleBoost);
                    temperature[index] =
                        std::lerp(temperature[index], 0.82, jungleBoost);
                }
            }
        });
}

int Random::getPoolIndex(int size, std::source_location origin)
{
    std::string key = std::to_string(origin.line()) + ':' +
                      std::to_string(origin.column()) + ':' +
                      origin.function_name();
    if (!poolState.contains(key)) {
        poolState[key] = getInt(0, size - 1);
    }
    ++poolState[key];
    return poolState[key] % size;
}

void Random::shuffleNoise()
{
    noiseDeltaX = getInt(0, noiseWidth);
    noiseDeltaY = getInt(0, noiseHeight);
}

void Random::saveShuffleState()
{
    savedNoiseDeltaX = noiseDeltaX;
    savedNoiseDeltaY = noiseDeltaY;
}

void Random::restoreShuffleState()
{
    noiseDeltaX = savedNoiseDeltaX;
    noiseDeltaY = savedNoiseDeltaY;
}

bool Random::getBool()
{
    return getInt(0, 1) == 0;
}

uint8_t Random::getByte()
{
    return getInt(0, 0xff);
}

double Random::getDouble(double min, double max)
{
    std::uniform_real_distribution<> dist(min, max);
    return dist(rnd);
}

int Random::getInt(int min, int max)
{
    std::uniform_int_distribution<> dist(min, max);
    return dist(rnd);
}

double Random::getBlurNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return blurNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getCoarseNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return coarseNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getFineNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return fineNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getHumidity(int x, int y) const
{
    return x < 0 || y < 0 || x >= noiseWidth || y >= noiseHeight
               ? 0
               : humidity[noiseHeight * x + y];
}

double Random::getTemperature(int x, int y) const
{
    return x < 0 || y < 0 || x >= noiseWidth || y >= noiseHeight
               ? 0
               : temperature[noiseHeight * x + y];
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
