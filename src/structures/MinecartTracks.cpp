#include "structures/MinecartTracks.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>
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

inline const std::set<int> trackClearTiles{
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
    TileID::ebonsand,
    TileID::silt,
    TileID::snow,
    TileID::ice,
    TileID::thinIce,
    TileID::corruptIce,
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
    TileID::lavaMossStone,
    TileID::sandstone,
    TileID::hardenedSand,
    TileID::hardenedEbonsand,
    TileID::hardenedCrimsand,
    TileID::ebonsandstone,
    TileID::crimsandstone,
    TileID::desertFossil,
    TileID::kryptonMossStone,
    TileID::xenonMossStone,
    TileID::argonMossStone,
    TileID::neonMossStone,
    TileID::ashGrass,
    TileID::corruptJungleGrass,
    TileID::crimsonJungleGrass,
};

std::vector<Point> planTrack(Random &rnd, World &world)
{
    Point pos{
        rnd.getInt(0, world.getWidth()),
        rnd.getInt(world.getUndergroundLevel(), world.getUnderworldLevel())};
    Point delta{rnd.select({-1, 1}), 0};
    int maxLen = rnd.getInt(world.getWidth() / 26, world.getWidth() / 4.2);
    size_t minLen = std::min(120 + world.getWidth() / 70, maxLen - 10);
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

int findGrassInColumn(int x, int y, World &world)
{
    std::set<int> grasses{
        TileID::jungleGrass,
        TileID::mushroomGrass,
        TileID::corruptJungleGrass,
        TileID::crimsonJungleGrass};
    for (int swapJ = 0; swapJ < 100; ++swapJ) {
        int j = swapJ / 2;
        int minI = -j / 4;
        if (swapJ % 2 == 0) {
            j = -j;
        }
        for (int i = minI; i < 1 - minI; ++i) {
            Tile &tile = world.getTile(x + i, y + j);
            if (grasses.contains(tile.blockID)) {
                return tile.blockID;
            }
        }
    }
    return TileID::mud;
}

void applyMudGrass(int x, int y, World &world)
{
    for (int j = -10; j < 2; ++j) {
        Tile &tile = world.getTile(x, y + j);
        if (tile.blockID != TileID::mud || !world.isExposed(x, y + j)) {
            continue;
        }
        tile.blockID = findGrassInColumn(x, y + j, world);
    }
}

void applyTrackSupport(int x, int y, World &world)
{
    std::set<int> allowed{TileID::platform, TileID::woodenBeam};
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
    int numTracks =
        world.getWidth() * world.getHeight() / rnd.getInt(640000, 960000);
    while (numTracks > 0) {
        auto track = planTrack(rnd, world);
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
                applyMudGrass(x, y, world);
                continue;
            }
            tile.blockID = TileID::minecartTrack;
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
            applyMudGrass(x, y, world);
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
