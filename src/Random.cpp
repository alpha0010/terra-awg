#include "Random.h"

#include "Util.h"
#include "vendor/OpenSimplexNoise.hpp"
#include <iostream>

Random::Random()
    : noiseWidth(0), noiseHeight(0), noiseDeltaX(0), noiseDeltaY(0),
      rnd(std::random_device{}())
{
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
}

void Random::computeBlurNoise()
{
    std::cout << "Blurring noise\n";
    blurNoise.resize(coarseNoise.size());
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

void Random::shuffleNoise()
{
    noiseDeltaX = getInt(0, noiseWidth);
    noiseDeltaY = getInt(0, noiseHeight);
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
    return blurNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getCoarseNoise(int x, int y) const
{
    return coarseNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getFineNoise(int x, int y) const
{
    return fineNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
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
