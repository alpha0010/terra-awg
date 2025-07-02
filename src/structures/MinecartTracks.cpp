#include "structures/MinecartTracks.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

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

std::vector<Point>
planTrackAt(int x, int y, double lengthScale, Random &rnd, World &world)
{
    Point pos{x, y};
    Point delta{rnd.select({-1, 1}), 0};
    int hardMaxLen = world.getWidth() / 4.2;
    int maxLen = std::min(
        std::max<int>(
            lengthScale * rnd.getDouble(world.getWidth() / 26, hardMaxLen),
            20),
        hardMaxLen);
    size_t minLen =
        std::min<int>(lengthScale * (120 + world.getWidth() / 70), maxLen - 10);
    int minY = 0.8 * world.getUndergroundLevel();
    int maxY = 0.2 * world.getHeight() + 0.8 * world.getUnderworldLevel();
    std::vector<Point> track;
    for (int i = 0; i < maxLen; ++i) {
        double noise = rnd.getCoarseNoise(pos.first, pos.second);
        delta.second = noise > 0.2 ? 1 : noise < -0.2 ? -1 : 0;
        pos = addPts(pos, delta);
        if (pos.first < 50 || pos.first > world.getWidth() - 50 ||
            pos.second < minY || pos.second > maxY ||
            !world.regionPasses(
                pos.first,
                pos.second - 10,
                1,
                12,
                [](Tile &tile) {
                    return !tile.guarded &&
                           (!tile.actuator || tile.blockID != TileID::sand) &&
                           trackClearTiles.contains(tile.blockID);
                }) ||
            !world.regionPasses(
                pos.first,
                pos.second + 1,
                1,
                20,
                [](Tile &tile) {
                    return tile.blockID != TileID::minecartTrack;
                })) {
            break;
        }
        track.push_back(pos);
    }
    if (track.size() < minLen) {
        return {};
    }
    track.resize(track.size() - rnd.getInt(4, 10));
    if (delta.first == -1) {
        std::reverse(track.begin(), track.end());
    }
    return track;
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
            tile.blockID == TileID::dirt && y + j < world.getUndergroundLevel();
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
    constexpr auto allowed =
        frozen::make_set<int>({TileID::platform, TileID::woodenBeam});
    if (!allowed.contains(world.getTile(x - 1, y).blockID) ||
        !allowed.contains(world.getTile(x + 1, y).blockID)) {
        return;
    }
    Tile &tile = world.getTile(x, y);
    tile.wallID = WallID::Safe::palmWood;
    if (x % 7 != 0 || world.getTile(x, y + 1).blockID != TileID::empty) {
        return;
    }
    int supFloor = scanWhileEmpty({x, y + 1}, {0, 1}, world).second;
    ++supFloor;
    if (supFloor - y < 4 || supFloor - y > 24 ||
        !trackClearTiles.contains(world.getTile(x, supFloor).blockID)) {
        return;
    }
    tile.blockPaint = Paint::none;
    for (int beamY = y; beamY < supFloor; ++beamY) {
        world.getTile(x, beamY).blockID = TileID::woodenBeam;
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
        int noiseRow = track.front().second;
        for (size_t idx = 0; idx < track.size(); ++idx) {
            auto [x, y] = track[idx];
            int height = 7 + 2.9 * rnd.getFineNoise(2 * x, noiseRow);
            bool isEndpoint = idx == 0 || idx + 1 == track.size();
            if (isEndpoint) {
                height *= 0.3;
            } else if (idx == 1 || idx == track.size() - 2) {
                height *= 0.8;
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
            tile.blockPaint = Paint::none;
            Mode mode = idx + 2 == track.size()     ? Mode::none
                        : y > track[idx + 1].second ? Mode::asc
                        : y < track[idx + 1].second ? Mode::desc
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
                placePlatform(x, y + 1, Platform::stone, world);
                supportTile.blockPaint = Paint::brown;
                supportTile.guarded = true;
                applyTrackSupport(x - 1, y + 1, world);
            }
        }
        --numTracks;
    }
}
