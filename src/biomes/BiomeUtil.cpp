#include "BiomeUtil.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/map.h"
#include <algorithm>
#include <set>

double computeOreThreshold(double oreMultiplier)
{
    return (std::pow(std::max(0.0, oreMultiplier), 0.172) - 2.25614064334831) /
           1.9476581743693;
}

Point getEmbeddedPos(
    int x,
    int y,
    int deltaX,
    const std::set<int> &blocks,
    World &world)
{
    x += deltaX;
    for (int dist = 0; dist < 3 && blocks.contains(world.getTile(x, y).blockID);
         ++dist, x += deltaX) {
        if (world.regionPasses(x - 1, y - 1, 3, 3, [&blocks](Tile &tile) {
                return blocks.contains(tile.blockID);
            })) {
            if (world.regionPasses(
                    x + deltaX - 1,
                    y - 2,
                    3,
                    5,
                    [&blocks](Tile &tile) {
                        return blocks.contains(tile.blockID);
                    })) {
                return {x + deltaX, y};
            }
            return {x, y};
        }
    }
    return {-1, -1};
}

void embedWaterfalls(
    Point from,
    Point to,
    std::initializer_list<int> allowedBlocks,
    Liquid liquid,
    int proximity,
    Random &rnd,
    World &world)
{
    std::set<int> blocks{allowedBlocks.begin(), allowedBlocks.end()};
    std::vector<std::tuple<int, int, int>> waterSources;
    for (int y = from.y; y < to.y; ++y) {
        int state = 0;
        for (int x = from.x; x < to.x; ++x) {
            Tile &tile = world.getTile(x, y);
            int nextState = tile.blockID != TileID::empty ? -1
                            : tile.liquid != Liquid::none ? 0
                                                          : 1;
            if (state != 0 && nextState != 0 && state != nextState) {
                auto [sourceX, sourceY] =
                    getEmbeddedPos(x, y, state, blocks, world);
                if (sourceX != -1) {
                    waterSources.emplace_back(sourceX, sourceY, nextState);
                }
            }
            state = nextState;
        }
    }
    std::shuffle(waterSources.begin(), waterSources.end(), rnd.getPRNG());
    std::vector<Point> usedLocations;
    std::vector<Point> waterStream;
    for (auto [x, y, deltaX] : waterSources) {
        if (isLocationUsed(x, y, proximity, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        Tile &sourceTile = world.getTile(x, y);
        sourceTile.blockID = TileID::empty;
        sourceTile.liquid = liquid;
        for (int i = deltaX; world.getTile(x + i, y).blockID != TileID::empty;
             i += deltaX) {
            waterStream.emplace_back(x + i, y);
        }
    }
    world.queuedDeco.emplace_back(
        [blocks, waterStream](Random &, World &world) {
            for (auto [x, y] : waterStream) {
                Tile &tile = world.getTile(x, y);
                Tile &aboveTile = world.getTile(x, y - 1);
                if (blocks.contains(tile.blockID) &&
                    (aboveTile.blockID == TileID::empty ||
                     isSolidBlock(aboveTile.blockID))) {
                    tile.slope = Slope::half;
                }
            }
        });
}

void fillLargeWallGaps(Point from, Point to, Random &rnd, World &world)
{
    int maxY = std::min(to.y, world.getUnderworldLevel());
    for (int x = from.x; x < to.x; ++x) {
        for (int y = from.y; y < maxY; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.wallID == WallID::empty && rnd.getFineNoise(x, y) < 0.55) {
                // Most biome generation code coverts dirt walls to biome
                // specific variants.
                tile.wallID = WallID::Unsafe::dirt;
            }
        }
    }
}

Point findStoneCave(int yMin, int yMax, Random &rnd, World &world, int minSize)
{
    return findCave(yMin, yMax, rnd, world, minSize, {TileID::stone});
}

Point findCave(
    int yMin,
    int yMax,
    Random &rnd,
    World &world,
    int minSize,
    std::initializer_list<int> allowedBlocks)
{
    std::set<int> blocks{allowedBlocks.begin(), allowedBlocks.end()};
    for (int numTries = 0; numTries < 5000; ++numTries) {
        if (numTries % 100 == 99 && minSize > 3) {
            // Slowly reduce size requirements if finding a cave is taking too
            // long.
            --minSize;
        }
        ++numTries;
        int x = rnd.getInt(50, world.getWidth() - 50);
        int y = rnd.getInt(yMin, yMax);
        if (world.getTile(x, y).blockID != TileID::empty) {
            continue;
        }
        // Scan up.
        int caveRoof = y - 1;
        while (caveRoof > 0 &&
               world.getTile(x, caveRoof).blockID == TileID::empty) {
            --caveRoof;
        }
        if (!blocks.contains(world.getTile(x, caveRoof).blockID)) {
            continue;
        }
        // Scan down.
        int caveFloor = y + 1;
        while (caveFloor < world.getHeight() &&
               world.getTile(x, caveFloor).blockID == TileID::empty) {
            ++caveFloor;
        }
        if (!blocks.contains(world.getTile(x, caveFloor).blockID) ||
            caveFloor - caveRoof < minSize ||
            caveFloor - caveRoof > minSize + 50) {
            continue;
        }
        // Scan left.
        int left = x - 1;
        while (left > 0 && world.getTile(left, y).blockID == TileID::empty) {
            --left;
        }
        if (!blocks.contains(world.getTile(left, y).blockID)) {
            continue;
        }
        // Scan right.
        int right = x + 1;
        while (right < world.getWidth() &&
               world.getTile(right, y).blockID == TileID::empty) {
            ++right;
        }
        if (!blocks.contains(world.getTile(right, y).blockID) ||
            right - left < minSize || right - left > minSize + 50) {
            continue;
        }
        // Success.
        return {(left + right) / 2, (caveFloor + caveRoof) / 2};
    }
    return {-1, -1};
}

Point getHexCentroid(Point pt, int scale)
{
    return getHexCentroid(pt.x, pt.y, scale);
}

Point getHexCentroid(int x, int y, int scale)
{
    int targetX = x;
    int targetY = y;
    int minDist = std::numeric_limits<int>::max();
    double centralDist = scale * 2 / std::sqrt(3);
    double startRow = std::floor(y / centralDist);
    for (int col : {x / scale, x / scale + 1}) {
        for (double row : {startRow, startRow + 1}) {
            if (col % 2 == 0) {
                row += 0.5;
            }
            int testX = col * scale;
            int testY = row * centralDist;
            int testDist = std::pow(x - testX, 2) + std::pow(y - testY, 2);
            if (testDist < minDist) {
                targetX = testX;
                targetY = testY;
                minDist = testDist;
            }
        }
    }
    return {targetX, targetY};
}

inline constexpr auto mossFrameX = frozen::make_map<int, int>(
    {{TileID::greenMossStone, 0},
     {TileID::brownMossStone, 22},
     {TileID::redMossStone, 44},
     {TileID::blueMossStone, 66},
     {TileID::purpleMossStone, 88},
     {TileID::lavaMossStone, 110},
     {TileID::kryptonMossStone, 132},
     {TileID::xenonMossStone, 154},
     {TileID::argonMossStone, 176},
     {TileID::neonMossStone, 198},
     {TileID::heliumMossStone, 220}});

void growMossOn(int x, int y, World &world)
{
    Tile &baseTile = world.getTile(x, y);
    auto itr = mossFrameX.find(baseTile.blockID);
    if (itr == mossFrameX.end() || baseTile.slope != Slope::none) {
        return;
    }
    for (auto [i, j, frameY] :
         {std::tuple{0, -1, 0}, {0, 1, 54}, {1, 0, 108}, {-1, 0, 162}}) {
        Tile &tile = world.getTile(x + i, y + j);
        if (tile.blockID == TileID::empty && tile.liquid != Liquid::lava) {
            tile.blockID = TileID::mossPlant;
            tile.frameX = itr->second;
            tile.frameY = frameY + 18 * (fnv1a32pt(x + i, y + j) % 3);
        }
    }
}

bool isInBiome(int x, int y, int scanDist, Biome biome, World &world)
{
    double threshold = 2 * scanDist;
    threshold *= threshold;
    threshold *= 0.05;
    for (int i = -scanDist; i < scanDist; ++i) {
        for (int j = -scanDist; j < scanDist; ++j) {
            switch (biome) {
            case Biome::forest:
                threshold += world.getBiome(x + i, y + j).forest;
                break;
            case Biome::snow:
                threshold += world.getBiome(x + i, y + j).snow;
                break;
            case Biome::desert:
                threshold += world.getBiome(x + i, y + j).desert;
                break;
            case Biome::jungle:
                threshold += world.getBiome(x + i, y + j).jungle;
                break;
            case Biome::underworld:
                threshold += world.getBiome(x + i, y + j).underworld;
                break;
            }
            threshold -= 1;
            if (threshold < 0) {
                return false;
            }
        }
    }
    return true;
}

void iterateZone(
    Point start,
    World &world,
    std::function<bool(Point)> isValid,
    std::function<void(Point)> f)
{
    std::set<Point> border;
    std::set<Point> visited;
    std::vector<Point> locations{start};
    while (!locations.empty()) {
        Point loc = locations.back();
        locations.pop_back();
        if (loc.x < 0 || loc.y < 0 || loc.x >= world.getWidth() ||
            loc.y >= world.getHeight() || visited.contains(loc) ||
            border.contains(loc)) {
            continue;
        }
        if (isValid(loc)) {
            visited.insert(loc);
            for (auto delta : {Point{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                locations.push_back(loc + delta);
            }
        } else {
            border.insert(loc);
        }
    }
    for (Point loc : visited) {
        f(loc);
    }
}
