#include "Underworld.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "structures/data/Bridges.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

void copyTemplateTile(const Tile &from, Tile &to, int x, int y, Random &rnd)
{
    if (from.wallID != WallID::empty) {
        to.wallID =
            from.wallID == WallID::Safe::cloud ? WallID::empty : from.wallID;
        to.wallPaint = from.wallPaint;
    }
    if (from.blockID != TileID::empty) {
        to.blockID = from.blockID;
        if (to.blockID == TileID::obsidianBrick &&
            static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 7 == 0) {
            to.blockID = TileID::hellstoneBrick;
        }
        to.blockPaint = from.blockPaint;
        to.frameX = from.frameX;
        to.frameY = from.frameY;
        to.slope = from.slope;
        to.liquid = from.liquid;
    }
}

void identifyGroup(
    Point dest,
    int group,
    std::set<int> &groundedGroups,
    std::map<Point, std::pair<Point, int>> &bridgeData,
    World &world)
{
    auto itr = bridgeData.find(dest);
    if (itr == bridgeData.end()) {
        if (world.getTile(dest).blockID != TileID::empty) {
            groundedGroups.insert(group);
        }
        return;
    } else if (itr->second.second != -1) {
        return;
    }
    itr->second.second = group;
    for (auto delta : {Point{0, 1}, {0, -1}, {1, 0}, {-1, 0}}) {
        identifyGroup(dest + delta, group, groundedGroups, bridgeData, world);
    }
}

void addBridges(int centerLevel, int lavaLevel, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    TileBuffer bridge = Data::getBridge(world.getFramedTiles());
    int bridgeLevel = std::midpoint(centerLevel, lavaLevel);
    auto shouldIncludePt = [&rnd](int x, int y) {
        double threshold = rnd.getCoarseNoise(x, y) > -0.15 ? -0.3 : 0.25;
        return rnd.getFineNoise(x, y) < threshold;
    };
    std::map<Point, std::pair<Point, int>> bridgeData;
    int skipFrom =
        world.conf.dontDigUp
            ? makeCongruent(0.39 * world.getWidth() - 10, bridge.getWidth())
            : -1;
    int skipTo =
        world.conf.dontDigUp
            ? makeCongruent(0.61 * world.getWidth() + 10, bridge.getWidth())
            : -1;
    for (int x = 0; x < world.getWidth(); x += bridge.getWidth()) {
        if (x == skipFrom) {
            x = skipTo;
        }
        for (int i = 0; i < bridge.getWidth(); ++i) {
            for (int j = 0; j < bridge.getHeight(); ++j) {
                Tile &bridgeTile = bridge.getTile(i, j);
                if (bridgeTile.blockID == TileID::empty &&
                    bridgeTile.wallID == WallID::empty) {
                    continue;
                }
                if (j < bridge.getHeight() - 1) {
                    Tile &tile = world.getTile(x + i, bridgeLevel + j);
                    if ((tile.blockID == TileID::empty ||
                         (world.conf.hiveQueen && tile.flag == Flag::border)) &&
                        shouldIncludePt(x + i, bridgeLevel + j)) {
                        bridgeData[{x + i, bridgeLevel + j}] = {{i, j}, -1};
                    }
                } else {
                    for (int y = bridgeLevel + j; y < world.getHeight(); ++y) {
                        Tile &tile = world.getTile(x + i, y);
                        if (tile.blockID != TileID::empty) {
                            if (y < lavaLevel) {
                                continue;
                            } else {
                                break;
                            }
                        }
                        if (shouldIncludePt(x + i, y)) {
                            bridgeData[{x + i, y}] = {{i, j}, -1};
                        }
                    }
                }
            }
        }
    }
    std::set<int> groundedGroups;
    int nextGroup = 0;
    for (auto &[dest, source] : bridgeData) {
        if (source.second == -1) {
            identifyGroup(dest, nextGroup, groundedGroups, bridgeData, world);
            ++nextGroup;
        }
        if (!groundedGroups.contains(source.second)) {
            continue;
        }
        Tile &tile = world.getTile(dest);
        Tile &bridgeTile = bridge.getTile(source.first.x, source.first.y);
        copyTemplateTile(bridgeTile, tile, dest.x, dest.y, rnd);
        tile.guarded = dest.y < lavaLevel;
        if (bridgeTile.blockID == TileID::lamp) {
            int offset = (bridgeTile.frameY % 54) / 18;
            for (int j = 0; j < 4; ++j) {
                if (!bridgeData.contains({dest.x, dest.y + j - offset})) {
                    tile.blockID = TileID::empty;
                    tile.frameX = 0;
                    tile.frameY = 0;
                    break;
                }
            }
        }
    }
}

void genUnderworld(Random &rnd, World &world)
{
    std::cout << "Igniting the depths\n";
    rnd.shuffleNoise();
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int centerLevel = world.getUnderworldLevel() + 0.32 * underworldHeight;
    double upperDist = centerLevel - world.getUnderworldLevel();
    double lowerDist = world.getHeight() - centerLevel;
    int lavaLevel = world.getUnderworldLevel() + 0.46 * underworldHeight;
    double aspectRatio = std::min(
        static_cast<double>(world.getHeight()) / world.getWidth(),
        0.5);
    auto getIslandSurface = [lavaLevel, &rnd, &world](int x) {
        if (!world.conf.dontDigUp) {
            return world.getHeight();
        }
        double threshold = std::clamp(
            100 * std::abs(x - 0.5 * world.getWidth()) / world.getWidth() - 10,
            0.0,
            1.0);
        return static_cast<int>(std::lerp(
            lavaLevel - 10 + 5 * rnd.getFineNoise(x, 0),
            world.getHeight(),
            threshold));
    };
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [aspectRatio,
         centerLevel,
         lavaLevel,
         upperDist,
         lowerDist,
         &getIslandSurface,
         &rnd,
         &world](int x) {
            int stalactiteLen =
                std::max(0.0, 16 * rnd.getFineNoise(4 * x, aspectRatio * x));
            bool foundRoof = false;
            for (int y = centerLevel;
                 y > world.getUnderworldLevel() && stalactiteLen < 10;
                 --y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::ash) {
                    foundRoof = true;
                }
                if (foundRoof) {
                    if (tile.blockID == TileID::ash) {
                        tile.blockID = TileID::empty;
                        ++stalactiteLen;
                    } else {
                        break;
                    }
                }
            }
            int islandSurface = getIslandSurface(x);
            for (int y = world.getUnderworldLevel(); y < world.getHeight();
                 ++y) {
                Tile &tile = world.getTile(x, y);
                if (y > islandSurface) {
                    tile.blockID = TileID::ash;
                    continue;
                } else if (
                    islandSurface < lavaLevel &&
                    tile.blockID == TileID::hellstone) {
                    tile.blockID = TileID::ash;
                }
                double threshold =
                    0.25 - 0.25 * (y < centerLevel
                                       ? (centerLevel - y) / upperDist
                                       : (y - centerLevel) / lowerDist);
                if (tile.blockID == TileID::hive &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            7 ==
                        0) {
                    tile.blockID = TileID::crispyHoney;
                }
                if (std::abs(rnd.getCoarseNoise(2 * x, y + aspectRatio * x)) <
                    threshold) {
                    for (auto [i, j] : {std::pair{-1, -1}, {-1, 0}, {0, -1}}) {
                        Tile &prevTile = world.getTile(x + i, y + j);
                        if (prevTile.blockID == TileID::mud) {
                            prevTile.blockID = TileID::jungleGrass;
                        }
                    }
                    tile.blockID = TileID::empty;
                    tile.wallID = WallID::empty;
                } else if (
                    std::abs(rnd.getFineNoise(2 * x, 2 * y + aspectRatio * x)) <
                        0.12 ||
                    std::abs(rnd.getFineNoise(3 * x, 3 * y + aspectRatio * x)) <
                        0.06) {
                    tile.wallID = WallID::empty;
                }
                if (y > lavaLevel) {
                    if (tile.blockID == TileID::empty) {
                        tile.liquid = Liquid::lava;
                    }
                } else if (
                    tile.blockID == TileID::mud && world.isExposed(x, y)) {
                    tile.blockID = TileID::jungleGrass;
                }
            }
        });
    addBridges(centerLevel, lavaLevel, rnd, world);
    int skipFrom = 0.15 * world.getWidth();
    int skipTo = 0.85 * world.getWidth();
    int minX = world.conf.dontDigUp ? 0.39 * world.getWidth() : 0;
    int maxX =
        world.conf.dontDigUp ? 0.61 * world.getWidth() : world.getWidth();
    std::vector<Point> spawnOpts;
    for (int x = minX; x < maxX; ++x) {
        if (x == skipFrom) {
            x = skipTo;
        }
        for (int y = world.getUnderworldLevel() - 50; y < lavaLevel; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::ash && world.isExposed(x, y)) {
                tile.blockID = TileID::ashGrass;
                if (world.conf.spawn == SpawnPoint::underworld &&
                    y > centerLevel && x > 150 && x < world.getWidth() - 150) {
                    spawnOpts.emplace_back(x, y - 1);
                }
            }
        }
    }
    if (world.conf.dontDigUp && spawnOpts.size() > 1002) {
        spawnOpts.erase(
            spawnOpts.begin(),
            spawnOpts.begin() + (spawnOpts.size() / 2 - 500));
        spawnOpts.resize(1000);
    }
    std::shuffle(spawnOpts.begin(), spawnOpts.end(), rnd.getPRNG());
    for (auto [x, y] : spawnOpts) {
        int numEmpty = 0;
        if (world.regionPasses(
                x - 7,
                y - 4,
                15,
                1,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(
                x - 3,
                y - 3,
                7,
                4,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(x - 5, y + 1, 11, 2, [&numEmpty](Tile &tile) {
                if (tile.blockID == TileID::empty) {
                    ++numEmpty;
                }
                return numEmpty < 4;
            })) {
            world.spawn = {x, y};
            break;
        }
    }
}
