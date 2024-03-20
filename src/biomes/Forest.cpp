#include "Forest.h"

#include "Random.h"
#include "World.h"
#include <cmath>

namespace shape
{

class Sentinel
{
};

template <typename Shape> class ShapeIterator
{
public:
    ShapeIterator(const Shape &s, int w, int h)
        : x(0), y(-1), width(w), height(h), geom(s)
    {
        ++(*this);
    }

    ShapeIterator &operator++()
    {
        do {
            ++y;
            if (y >= height) {
                y = 0;
                ++x;
            }
        } while (x < width && !geom.includes(x, y));
        return *this;
    }

    ShapeIterator operator++(int) = delete;

    bool operator==(Sentinel) const
    {
        return x >= width;
    }

    std::pair<int, int> operator*()
    {
        return {x, y};
    }

private:
    int x;
    int y;
    int width;
    int height;
    const Shape &geom;
};

class Deposit
{
public:
    Deposit(Random &rnd) : size(20)
    {
        for (int i = 0; i < 9; ++i) {
            spokes.push_back(rnd.getDouble(0, size / 2.0));
        }
        double prev = spokes.back();
        for (auto &spoke : spokes) {
            double cur = spoke;
            spoke = (cur + prev) / 2;
            prev = cur;
        }
    }

    bool includes(int x, int y) const
    {
        double midX = size / 2.0;
        double midY = size / 2.0;
        double angle = std::atan2(y - midY, x - midX);
        double spoke = spokes[std::floor(
            spokes.size() * angle / (2 * std::numbers::pi) + 0.5)];
        return std::hypot(x - midX, y - midY) < spoke;
    }

    ShapeIterator<Deposit> begin() const
    {
        return {*this, size, size};
    }

    Sentinel end() const
    {
        return {};
    }

private:
    int size;
    std::vector<double> spokes;
};

} // namespace shape

void genForest(Random &rnd, World &world)
{
    double surfaceAngle = 0;
    double surfaceLevel = rnd.getDouble(
        0.7 * world.getUndergroundLevel(),
        0.8 * world.getUndergroundLevel());
    double boundMin = 0.55 * world.getUndergroundLevel();
    double boundMax = 0.95 * world.getUndergroundLevel();
    double boundRange = boundMax - boundMin;
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = surfaceLevel; y < world.getUndergroundLevel(); ++y) {
            Tile &tile = world.getTile(x, y);
            tile.blockID = TileID::dirt;
        }

        surfaceAngle *= 0.96;
        if (rnd.getDouble(0, 1) > 0.72) {
            double ratio = (surfaceLevel - boundMin) / boundRange;
            double bias = rnd.getDouble(0, 1) - ratio;

            if (bias < -0.12) {
                surfaceAngle = std::max(
                    surfaceAngle - rnd.getDouble(0.1, 0.35),
                    -0.45 * std::numbers::pi);
            } else if (bias > 0.12) {
                surfaceAngle = std::min(
                    surfaceAngle + rnd.getDouble(0.1, 0.35),
                    0.45 * std::numbers::pi);
            } else {
                surfaceAngle *= 0.9;
            }
        }
        surfaceLevel += std::tan(surfaceAngle);
    }

    for (int yMin = boundMin; yMin < world.getUndergroundLevel(); yMin += 10) {
        for (int x = rnd.getInt(0, 10); x < world.getWidth();
             x += rnd.getInt(10, 100)) {
            int y = rnd.getInt(yMin, world.getUndergroundLevel());
            for (auto [i, j] : shape::Deposit(rnd)) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::dirt) {
                    tile.blockID = TileID::stone;
                }
            }
        }
    }
}
