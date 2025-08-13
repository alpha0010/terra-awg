#include "structures/MinecartTracks.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <numbers>
#include <set>

namespace Track
{
enum {
    flat = 1,
    flatEndLeft = 2,
    flatEndRight = 3,
    ascToFlat = 4,
    flatToDesc = 5,
    flatToAsc = 6,
    descToFlat = 7,
    desc = 8,
    asc = 9,
    ascEndRight = 10,
    descEndLeft = 11,
    descEndRight = 12,
    ascEndLeft = 13,
};
}

enum class Mode { none, flat, asc, desc };

inline constexpr auto trackClearTiles = frozen::make_set<int>({
    TileID::empty,
    TileID::dirt,
    TileID::stone,
    TileID::grass,
    TileID::ironOre,
    TileID::copperOre,
    TileID::goldOre,
    TileID::silverOre,
    TileID::demonite,
    TileID::corruptGrass,
    TileID::clay,
    TileID::cobweb,
    TileID::sand,
    TileID::ash,
    TileID::hellstone,
    TileID::mud,
    TileID::jungleGrass,
    TileID::mushroomGrass,
    TileID::hallowedGrass,
    TileID::ebonsand,
    TileID::pearlsand,
    TileID::pearlstone,
    TileID::silt,
    TileID::snow,
    TileID::ice,
    TileID::thinIce,
    TileID::corruptIce,
    TileID::hallowedIce,
    TileID::tinOre,
    TileID::leadOre,
    TileID::tungstenOre,
    TileID::platinumOre,
    TileID::gem,
    TileID::corruptIce,
    TileID::slime,
    TileID::crimsonGrass,
    TileID::crimsonIce,
    TileID::crimtane,
    TileID::slush,
    TileID::crimsand,
    TileID::crispyHoney,
    TileID::smoothMarble,
    TileID::marble,
    TileID::granite,
    TileID::smoothGranite,
    TileID::pinkSlime,
    TileID::lavaMossStone,
    TileID::sandstone,
    TileID::hardenedSand,
    TileID::hardenedEbonsand,
    TileID::hardenedCrimsand,
    TileID::ebonsandstone,
    TileID::crimsandstone,
    TileID::hardenedPearlsand,
    TileID::pearlsandstone,
    TileID::desertFossil,
    TileID::kryptonMossStone,
    TileID::xenonMossStone,
    TileID::argonMossStone,
    TileID::neonMossStone,
    TileID::ashGrass,
    TileID::corruptJungleGrass,
    TileID::crimsonJungleGrass,
    TileID::cobaltOre,
    TileID::palladiumOre,
    TileID::mythrilOre,
    TileID::orichalcumOre,
    TileID::adamantiteOre,
    TileID::titaniumOre,
    TileID::chlorophyteOre,
});

bool canTrackClearTile(Tile &tile)
{
    return trackClearTiles.contains(tile.blockID) ||
           (tile.blockID == TileID::hive && tile.flag != Flag::none);
}

inline std::array gemsparkRainbow{
    WallID::Safe::rubyGemspark,
    WallID::Safe::amberGemspark,
    WallID::Safe::topazGemspark,
    WallID::Safe::emeraldGemspark,
    WallID::Safe::sapphireGemspark,
    WallID::Safe::amethystGemspark};

int getRainbowGemsparkWall(int x, int y)
{
    return gemsparkRainbow[(x + y) % gemsparkRainbow.size()];
}

std::pair<size_t, int>
computeTrackBounds(double lengthScale, Random &rnd, World &world)
{
    int hardMaxLen = world.getWidth() / 4.2;
    int maxLen = std::min(
        std::max<int>(
            lengthScale * rnd.getDouble(world.getWidth() / 26, hardMaxLen),
            20),
        hardMaxLen);
    size_t minLen =
        std::min<int>(lengthScale * (120 + world.getWidth() / 70), maxLen - 10);
    return {minLen, maxLen};
}

bool isValidTrackSegment(int x, int y, World &world)
{
    int minY = 0.8 * world.getUndergroundLevel();
    int maxY = 0.2 * world.getHeight() + 0.8 * world.getUnderworldLevel();
    return x > 50 && x < world.getWidth() - 50 && y > minY && y < maxY &&
           world.regionPasses(
               x,
               y - 10,
               1,
               12,
               [](Tile &tile) {
                   return !tile.guarded &&
                          (!tile.actuator || tile.blockID != TileID::sand) &&
                          canTrackClearTile(tile);
               }) &&
           world.regionPasses(x, y + 1, 1, 20, [](Tile &tile) {
               return tile.blockID != TileID::minecartTrack;
           });
}

std::vector<Point>
planStandardTrackAt(int x, int y, double lengthScale, Random &rnd, World &world)
{
    Point pos{x, y};
    Point delta{rnd.select({-1, 1}), 0};
    auto [minLen, maxLen] = computeTrackBounds(lengthScale, rnd, world);
    std::vector<Point> track;
    for (int i = 0; i < maxLen; ++i) {
        double noise = rnd.getCoarseNoise(pos.x, pos.y);
        delta.y = noise > 0.2 ? 1 : noise < -0.2 ? -1 : 0;
        pos += delta;
        if (!isValidTrackSegment(pos.x, pos.y, world)) {
            break;
        }
        track.push_back(pos);
    }
    if (track.size() < minLen) {
        return {};
    }
    track.resize(track.size() - rnd.getInt(4, 10));
    if (delta.x == -1) {
        std::reverse(track.begin(), track.end());
    }
    return track;
}

std::vector<Point>
planWavyTrackAt(int x, int y, double lengthScale, Random &rnd, World &world)
{
    Pointf pos{x, y};
    int delta = rnd.select({-1, 1});
    auto [minLen, maxLen] = computeTrackBounds(lengthScale, rnd, world);
    double noiseScale = rnd.getDouble(1.2, 3.8);
    double angle = std::numbers::pi *
                   rnd.getCoarseNoise(noiseScale * x, noiseScale * y) / 4;
    int prevY = y;
    int curY = y;
    std::vector<Point> track;
    for (int i = 0; i < maxLen; ++i) {
        double noise =
            rnd.getCoarseNoise(noiseScale * pos.x, noiseScale * pos.y);
        double turn = std::max(std::abs(noise) / 10, 0.015);
        angle = std::clamp(
            0.975 * angle + (noise > 0 ? turn : -turn),
            -std::numbers::pi / 4,
            std::numbers::pi / 4);
        pos.x += delta;
        pos.y += std::tan(angle);
        if (!isValidTrackSegment(pos.x, pos.y, world)) {
            break;
        }
        int nextY = std::clamp<int>(
            std::round(pos.y),
            prevY < curY ? curY : curY - 1,
            prevY > curY ? curY : curY + 1);
        prevY = curY;
        curY = nextY;
        track.emplace_back(std::round(pos.x), curY);
    }
    if (track.size() < minLen) {
        return {};
    }
    track.resize(track.size() - rnd.getInt(4, 10));
    if (delta == -1) {
        std::reverse(track.begin(), track.end());
    }
    return track;
}

std::vector<Point>
planTrackAt(int x, int y, double lengthScale, Random &rnd, World &world)
{
    return world.conf.celebration ||
                   (!world.conf.hiveQueen &&
                    hypot(world.aether, {x, y}) <
                        world.conf.aetherSize *
                            (80 +
                             world.getWidth() * world.getHeight() / 115200))
               ? planWavyTrackAt(x, y, lengthScale, rnd, world)
               : planStandardTrackAt(x, y, lengthScale, rnd, world);
}

std::vector<Point> planTrack(double lengthScale, Random &rnd, World &world)
{
    return planTrackAt(
        rnd.getInt(50, world.getWidth() - 50),
        rnd.getInt(world.getUndergroundLevel(), world.getUnderworldLevel()),
        lengthScale,
        rnd,
        world);
}

std::vector<Point> planSurfaceTrack(Random &rnd, World &world)
{
    if (world.conf.sunken) {
        return {};
    }
    std::vector<Point> locations;
    int minY = 0.8 * world.getUndergroundLevel();
    for (int x = 0.25 * world.getWidth(); x < 0.75 * world.getWidth(); ++x) {
        if (x == world.getWidth() / 2 - 150) {
            x = world.getWidth() / 2 + 150;
        }
        int y = world.getSurfaceLevel(x);
        if (y > minY && world.getTile(x, y - 1).liquid == Liquid::none) {
            locations.emplace_back(x, y);
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    for (auto [x, y] : locations) {
        auto track = planTrackAt(x, y, 1.0, rnd, world);
        if (!track.empty()) {
            return track;
        }
    }
    return {};
}

std::pair<int, int> findGrassInColumn(int x, int y, bool isDirt, World &world)
{
    constexpr auto dirtGrasses = frozen::make_set<int>(
        {TileID::grass,
         TileID::hallowedGrass,
         TileID::corruptGrass,
         TileID::crimsonGrass});
    constexpr auto mudGrasses = frozen::make_set<int>(
        {TileID::jungleGrass,
         TileID::mushroomGrass,
         TileID::corruptJungleGrass,
         TileID::crimsonJungleGrass});
    for (int swapJ = 0; swapJ < 100; ++swapJ) {
        int j = swapJ / 2;
        int minI = -j / 4;
        if (swapJ % 2 == 0) {
            j = -j;
        }
        for (int i = minI; i < 1 - minI; ++i) {
            Tile &tile = world.getTile(x + i, y + j);
            if (isDirt ? dirtGrasses.contains(tile.blockID)
                       : mudGrasses.contains(tile.blockID)) {
                return {tile.blockID, tile.blockPaint};
            }
        }
    }
    return {isDirt ? TileID::dirt : TileID::mud, Paint::none};
}

void applyTrackGrass(int x, int y, World &world)
{
    for (int j = -10; j < 2; ++j) {
        Tile &tile = world.getTile(x, y + j);
        bool isDirt =
            tile.blockID == TileID::dirt &&
            (y + j < world.getUndergroundLevel() || world.conf.dontDigUp);
        if ((!isDirt && tile.blockID != TileID::mud) ||
            !world.isExposed(x, y + j)) {
            continue;
        }
        std::tie(tile.blockID, tile.blockPaint) =
            findGrassInColumn(x, y + j, isDirt, world);
    }
}

void applyTrackSupport(int x, int y, World &world)
{
    constexpr auto allowed = frozen::make_set<int>(
        {TileID::platform, TileID::marbleColumn, TileID::woodenBeam});
    if (!allowed.contains(world.getTile(x - 1, y).blockID) ||
        !allowed.contains(world.getTile(x + 1, y).blockID)) {
        return;
    }
    Tile &tile = world.getTile(x, y);
    tile.wallID = world.conf.celebration ? getRainbowGemsparkWall(x, y)
                                         : WallID::Safe::palmWood;
    if (x % 7 != 0 || world.getTile(x, y + 1).blockID != TileID::empty) {
        return;
    }
    int supFloor = scanWhileEmpty({x, y + 1}, {0, 1}, world).y;
    ++supFloor;
    if (supFloor - y < 4 || supFloor - y > 24 ||
        !canTrackClearTile(world.getTile(x, supFloor))) {
        return;
    }
    tile.blockPaint = Paint::none;
    for (int beamY = y; beamY < supFloor; ++beamY) {
        if (world.conf.celebration) {
            Tile &beamTile = world.getTile(x, beamY);
            beamTile.blockID = TileID::marbleColumn;
            beamTile.blockPaint = getDeepRainbowPaint(x, beamY);
        } else {
            world.getTile(x, beamY).blockID = TileID::woodenBeam;
        }
    }
}

void clearTrackHex(Point centroid, World &world)
{
    iterateZone(
        centroid,
        world,
        [centroid](Point pt) { return getHexCentroid(pt, 10) == centroid; },
        [&world](Point pt) {
            Tile &prevTile = world.getTile(pt.x, pt.y - 1);
            if (!canTrackClearTile(prevTile) &&
                prevTile.blockID != TileID::minecartTrack &&
                prevTile.blockID != TileID::platform) {
                return;
            }
            Tile &tile = world.getTile(pt);
            if (!tile.guarded && canTrackClearTile(tile)) {
                tile.blockID = TileID::empty;
                tile.actuator = false;
            }
        });
    for (int i = -10; i < 10; ++i) {
        applyTrackGrass(centroid.x + i, centroid.y, world);
        applyTrackGrass(centroid.x + i, centroid.y + 10, world);
    }
}

void genTracks(Random &rnd, World &world)
{
    std::cout << "Installing tracks\n";
    rnd.shuffleNoise();
    int numTracks = world.conf.minecartTracks * world.getWidth() *
                    world.getHeight() / rnd.getInt(640000, 960000);
    int maxTries = 5000 * numTracks;
    for (int tries = 0; numTracks > 0 && tries < maxTries; ++tries) {
        auto track = tries == 0
                         ? planSurfaceTrack(rnd, world)
                         : planTrack(
                               world.conf.minecartLength > 1.0
                                   ? std::lerp(
                                         world.conf.minecartLength,
                                         1.0,
                                         static_cast<double>(tries) / maxTries)
                                   : world.conf.minecartLength,
                               rnd,
                               world);
        if (track.empty()) {
            continue;
        }
        Mode prevMode = Mode::none;
        int noiseRow = track.front().y;
        std::set<Point> clearedHexes;
        for (size_t idx = 0; idx < track.size(); ++idx) {
            auto [x, y] = track[idx];
            int height = 7 + 2.9 * rnd.getFineNoise(2 * x, noiseRow);
            bool isEndpoint = idx == 0 || idx + 1 == track.size();
            if (isEndpoint) {
                height *= 0.3;
            } else if (idx == 1 || idx == track.size() - 2) {
                height *= 0.8;
            }
            if (world.conf.hiveQueen) {
                Point centroid = getHexCentroid(x, y - height, 10);
                if (!clearedHexes.contains(centroid) && centroid.y + 4 < y) {
                    clearTrackHex(centroid, world);
                    clearedHexes.insert(centroid);
                }
            }
            for (int j = -height; j < 0; ++j) {
                Tile &tile = world.getTile(x, y + j);
                tile.blockID = TileID::empty;
                tile.actuator = false;
            }
            Tile &tile = world.getTile(x, y);
            if (isEndpoint) {
                tile.blockID = TileID::empty;
                applyTrackGrass(x, y, world);
                continue;
            }
            tile.blockID = TileID::minecartTrack;
            tile.blockPaint = world.conf.celebration
                                  ? getDeepRainbowPaint(x, tries)
                                  : Paint::none;
            Mode mode = idx + 2 == track.size() ? Mode::none
                        : y > track[idx + 1].y  ? Mode::asc
                        : y < track[idx + 1].y  ? Mode::desc
                                                : Mode::flat;
            switch (prevMode) {
            case Mode::none:
                tile.frameX = mode == Mode::asc    ? Track::ascEndLeft
                              : mode == Mode::desc ? Track::descEndLeft
                                                   : Track::flatEndLeft;
                break;
            case Mode::flat:
                tile.frameX = mode == Mode::none   ? Track::flatEndRight
                              : mode == Mode::flat ? Track::flat
                              : mode == Mode::asc  ? Track::flatToAsc
                                                   : Track::flatToDesc;
                break;
            case Mode::asc:
                tile.frameX = mode == Mode::none   ? Track::ascEndRight
                              : mode == Mode::flat ? Track::ascToFlat
                                                   : Track::asc;
                break;
            case Mode::desc:
                tile.frameX = mode == Mode::none   ? Track::descEndRight
                              : mode == Mode::flat ? Track::descToFlat
                                                   : Track::desc;
                break;
            }
            tile.frameY = -1;
            prevMode = mode;
            applyTrackGrass(x, y, world);
            Tile &supportTile = world.getTile(x, y + 1);
            if (tile.frameX == Track::flat &&
                supportTile.blockID == TileID::empty) {
                if (world.conf.celebration) {
                    placePlatform(x, y + 1, Platform::balloon, world);
                } else {
                    placePlatform(x, y + 1, Platform::stone, world);
                    supportTile.blockPaint = Paint::brown;
                }
                supportTile.guarded = true;
                applyTrackSupport(x - 1, y + 1, world);
            }
        }
        --numTracks;
    }
}
