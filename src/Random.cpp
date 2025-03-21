#include "Random.h"

#include "OpenSimplexNoise.hpp"
#include <iostream>
#include <ranges>
#include <thread>

template <std::ranges::input_range R, class UnaryFunc>
constexpr void parallelFor(R &&r, UnaryFunc f)
{
    std::vector<std::thread> pool;
    size_t numThreads = std::max(std::thread::hardware_concurrency(), 4u);
    for (size_t loopId = 0; loopId < numThreads; ++loopId) {
        pool.emplace_back([&r, &f, loopId, numThreads]() {
            auto itr = r.begin();
            for (size_t i = 0; i < loopId && itr != r.end(); ++i, ++itr)
                ;
            while (itr != r.end()) {
                f(*itr);
                for (size_t i = 0; i < numThreads && itr != r.end(); ++i, ++itr)
                    ;
            }
        });
    }
    for (auto &worker : pool) {
        worker.join();
    }
}

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
