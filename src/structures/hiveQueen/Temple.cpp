#include "structures/hiveQueen/Temple.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "structures/Temple.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

class HiveTemple
{
private:
    Random &rnd;
    World &world;
    double size;
    int roomSize;
    std::set<Point> connections;

    std::vector<Point> neighborCentroids(Point pt)
    {
        std::vector<Point> neighbors;
        for (auto [i, j] :
             {std::pair{0, -1.155 * roomSize},
              {0, 1.155 * roomSize},
              {-roomSize, -0.578 * roomSize},
              {-roomSize, 0.578 * roomSize},
              {roomSize, -0.578 * roomSize},
              {roomSize, 0.578 * roomSize}}) {
            neighbors.push_back(
                getHexCentroid(pt.first + i, pt.second + j, roomSize));
        }
        return neighbors;
    }

    void makeRoom(Point centroid)
    {
        std::map<Point, int> visitCount;
        iterateZone(
            centroid,
            world,
            [this, centroid](Point pt) {
                return getHexCentroid(pt, roomSize) == centroid;
            },
            [this, &visitCount](Point pt) {
                Tile &tile = world.getTile(pt);
                tile.blockID = TileID::lihzahrdBrick;
                tile.blockPaint = Paint::none;
                tile.wallID = WallID::Unsafe::lihzahrdBrick;
                for (int i = -2; i < 3; ++i) {
                    for (int j = -2; j < 3; ++j) {
                        Point probe = addPts(pt, {i, j});
                        visitCount[probe] += 1;
                        if (visitCount[probe] == 25) {
                            world.getTile(probe).blockID = TileID::empty;
                        }
                    }
                }
            });
    }

    bool hasHallway(Point a, Point b)
    {
        return connections.contains(addPts(a, b));
    }

    void makeHallway(Point a, Point b)
    {
        connections.insert(addPts(a, b));
        if (a.second > b.second) {
            std::swap(a, b);
        }
        double slope =
            static_cast<double>(b.first - a.first) / (b.second - a.second);
        double scanDist = 2.5 * (1 + std::abs(slope));
        for (int y = a.second; y <= b.second; ++y) {
            double centerX = slope * (y - a.second) + a.first;
            for (int x = std::round(centerX - scanDist); x < centerX + scanDist;
                 ++x) {
                world.getTile(x, y).blockID = TileID::empty;
            }
        }
    }

    void genMaze(Point center)
    {
        std::vector<Point> coreRing = neighborCentroids(center);
        for (Point pt : coreRing) {
            makeRoom(pt);
        }
        double coreSize = 1 + 1.22 * roomSize;
        for (int i = -coreSize; i < coreSize; ++i) {
            for (int j = -coreSize; j < coreSize; ++j) {
                if (std::hypot(i, j) < coreSize) {
                    Tile &tile =
                        world.getTile(center.first + i, center.second + j);
                    tile.blockID = TileID::empty;
                    tile.blockPaint = Paint::none;
                    tile.wallID = WallID::Unsafe::lihzahrdBrick;
                }
            }
        }
        std::set<Point> core(coreRing.begin(), coreRing.end());
        core.insert(center);
        Point agent = rnd.select(coreRing);
        std::set<Point> connectedRooms{agent};
        std::vector<Point> junctions;
        double threshold = size - roomSize;
        while (true) {
            std::vector<Point> choices;
            for (Point candidate : neighborCentroids(agent)) {
                if (std::hypot(
                        candidate.first - center.first,
                        candidate.second - center.second) < threshold &&
                    !core.contains(candidate) &&
                    (!connectedRooms.contains(candidate) ||
                     hasHallway(agent, candidate))) {
                    choices.push_back(candidate);
                }
            }
            if (choices.size() == 1 && connectedRooms.contains(choices[0])) {
                if (junctions.empty()) {
                    break;
                }
                agent = junctions.back();
                junctions.pop_back();
            } else {
                for (Point choice : choices) {
                    if (!connectedRooms.contains(choice)) {
                        junctions.push_back(agent);
                        break;
                    }
                }
                Point next = rnd.select(choices);
                if (!connectedRooms.contains(next)) {
                    makeRoom(next);
                    makeHallway(agent, next);
                    connectedRooms.insert(next);
                }
                agent = next;
            }
        }
        std::vector<Point> border;
        for (int x = center.first - size; x < center.first + size; ++x) {
            for (int y = center.second - size; y < center.second + size; ++y) {
                if (world.getTile(x, y).wallID !=
                        WallID::Unsafe::lihzahrdBrick &&
                    !world.regionPasses(x - 2, y - 2, 5, 5, [](Tile &tile) {
                        return tile.wallID != WallID::Unsafe::lihzahrdBrick;
                    })) {
                    border.emplace_back(x, y);
                }
            }
        }
        for (Point pt : border) {
            Tile &tile = world.getTile(pt);
            tile.blockID = TileID::lihzahrdBrick;
            tile.blockPaint = Paint::none;
            tile.wallID = WallID::Unsafe::lihzahrdBrick;
        }
    }

    void addEntries(Point center)
    {
        int x = center.first - size;
        int y = center.second;
        while (world.getTile(x, y).blockID != TileID::lihzahrdBrick) {
            ++x;
        }
        std::tie(x, y) = getHexCentroid(x + roomSize / 2, y, roomSize);
        clearTempleSurface(
            {x - roomSize / 2, y - roomSize / 2},
            1.1 * roomSize,
            rnd,
            world);
        for (bool inTemple = true; inTemple; --x) {
            inTemple = false;
            for (int j = -1; j < 2; ++j) {
                Tile &tile = world.getTile(x, y + j);
                if (tile.wallID == WallID::Unsafe::lihzahrdBrick) {
                    tile.blockID = TileID::empty;
                    inTemple = true;
                }
            }
        }
        world.placeFramedTile(x + 2, y - 1, TileID::door, Variant::lihzahrd);
        world.placeFramedTile(x + 4, y, TileID::pot, Variant::lihzahrd);

        x = 2 * center.first - x;
        while (world.getTile(x, y).blockID != TileID::lihzahrdBrick) {
            --x;
        }
        std::tie(x, y) = getHexCentroid(x - roomSize / 2, y, roomSize);
        clearTempleSurface(
            {x + roomSize / 2, y - roomSize / 2},
            1.1 * roomSize,
            rnd,
            world);
        for (bool inTemple = true; inTemple; ++x) {
            inTemple = false;
            for (int j = -1; j < 2; ++j) {
                Tile &tile = world.getTile(x, y + j);
                if (tile.wallID == WallID::Unsafe::lihzahrdBrick) {
                    tile.blockID = TileID::empty;
                    inTemple = true;
                }
            }
        }
        world.placeFramedTile(x - 2, y - 1, TileID::door, Variant::lihzahrd);
        world.placeFramedTile(x - 5, y, TileID::pot, Variant::lihzahrd);
    }

    void placeAltar(Point center)
    {
        int altarLeft = scanWhileEmpty(
                            {center.first - roomSize, center.second},
                            {0, 1},
                            world)
                            .second;
        int altarCenter = scanWhileEmpty(center, {0, 1}, world).second;
        world.placeFramedTile(
            center.first - 1 - (altarLeft < altarCenter ? roomSize : 0),
            std::min(altarLeft, altarCenter) - 1,
            TileID::lihzahrdAltar);
    }

    std::vector<Point> placeTreasures(Point center)
    {
        std::vector<Point> locations;
        for (int x = center.first - size; x < center.first + size; ++x) {
            for (int y = center.second - size; y < center.second + size; ++y) {
                if (canPlaceTempleTreasureAt(x, y, world)) {
                    locations.emplace_back(x, y);
                }
            }
        }
        addTempleTreasures(locations, size * size / 127.7 + 20, rnd, world);
        return locations;
    }

    void addSpikes(Point center)
    {
        for (int x = center.first - size; x < center.first + size; ++x) {
            for (int y = center.second - size; y < center.second + size; ++y) {
                if (world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                        return tile.blockID == TileID::empty &&
                               tile.wallID == WallID::Unsafe::lihzahrdBrick;
                    })) {
                    addTempleSpikesAt({x, y}, rnd, world);
                }
            }
        }
    }

    void applyPaint(Point center, int paint)
    {
        for (int x = center.first - size; x < center.first + size; ++x) {
            for (int y = center.second - size; y < center.second + size; ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.wallID == WallID::Unsafe::lihzahrdBrick) {
                    if (tile.blockID == TileID::lihzahrdBrick ||
                        tile.blockID == TileID::pressurePlate ||
                        tile.blockID == TileID::trap) {
                        tile.blockPaint = paint;
                    }
                    tile.wallPaint = paint;
                }
            }
        }
    }

public:
    HiveTemple(double s, Random &r, World &w)
        : rnd(r), world(w), size(s), roomSize(20)
    {
    }

    void gen(Point center)
    {
        center = getHexCentroid(center, roomSize);
        genMaze(center);
        addEntries(center);
        placeAltar(center);
        std::vector<Point> flatLocations = placeTreasures(center);
        std::shuffle(flatLocations.begin(), flatLocations.end(), rnd.getPRNG());
        addTempleTraps(flatLocations, 45, rnd, world);
        addSpikes(center);
        applyPaint(
            center,
            rnd.select(
                {Paint::yellow,
                 Paint::orange,
                 Paint::deepYellow,
                 Paint::deepOrange}));
    }
};

void genTempleHiveQueen(Random &rnd, World &world)
{
    if (world.conf.templeSize < 0.01) {
        return;
    }
    std::cout << "Training acolytes\n";
    rnd.shuffleNoise();
    double size = std::max(
        world.conf.templeSize * 0.022 *
            std::midpoint<double>(world.getWidth(), 3.55 * world.getHeight()),
        95.0);
    Point center = selectTempleCenter(
        [scanDist = 0.95 * size](Point center, World &world) {
            constexpr auto avoidBlocks = frozen::make_set<int>({
                TileID::aetherium,
                TileID::ash,
                TileID::blueBrick,
                TileID::corruptJungleGrass,
                TileID::crimsonJungleGrass,
                TileID::granite,
                TileID::greenBrick,
                TileID::hive,
                TileID::marble,
                TileID::mushroomGrass,
                TileID::pinkBrick,
            });
            return world.regionPasses(
                center.first - scanDist,
                center.second - scanDist,
                2 * scanDist,
                2 * scanDist,
                [&avoidBlocks](Tile &tile) {
                    return !avoidBlocks.contains(tile.blockID) ||
                           (tile.blockID == TileID::hive && tile.flag == 1);
                });
        },
        rnd,
        world);
    if (center.first < 100) {
        return;
    }
    HiveTemple structure(size, rnd, world);
    structure.gen(center);
}
