#include "structures/beamMeUp/Teleporters.h"

#include "Config.h"
#include "Pathfinder.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

namespace
{
constexpr auto wireAvoidTiles = frozen::make_set<int>({
    TileID::amberGemspark,
    TileID::amethystGemspark,
    TileID::candelabra,
    TileID::candle,
    TileID::chandelier,
    TileID::chest,
    TileID::chestGroup2,
    TileID::conveyorBeltCCW,
    TileID::conveyorBeltCW,
    TileID::detonator,
    TileID::diamondGemspark,
    TileID::door,
    TileID::emeraldGemspark,
    TileID::explosives,
    TileID::grate,
    TileID::junctionBox,
    TileID::lamp,
    TileID::lantern,
    TileID::offlineAmethystGemspark,
    TileID::offlineRubyGemspark,
    TileID::offlineSapphireGemspark,
    TileID::pressurePlate,
    TileID::rubyGemspark,
    TileID::sapphireGemspark,
    TileID::sensor,
    TileID::statue,
    TileID::teleporter,
    TileID::topazGemspark,
    TileID::torch,
    TileID::trap,
});

constexpr auto wireAvoidWalls = frozen::make_set([]() {
    auto walls = std::to_array<int>({
        WallID::Safe::amberGemspark,
        WallID::Safe::amethystGemspark,
        WallID::Safe::diamondGemspark,
        WallID::Safe::emeraldGemspark,
        WallID::Safe::rubyGemspark,
        WallID::Safe::sandstoneBrick,
        WallID::Safe::sapphireGemspark,
        WallID::Safe::topazGemspark,
        WallID::Unsafe::lihzahrdBrick,
    });
    std::array<int, walls.size() + WallVariants::dungeon.size()> ret;
    auto itr = std::copy(walls.begin(), walls.end(), ret.begin());
    std::copy(WallVariants::dungeon.begin(), WallVariants::dungeon.end(), itr);
    return ret;
}());
} // namespace

bool isTeleporterLocation(int x, int y, int minX, int maxX, World &world)
{
    return x > minX && y > 65 && x < maxX && y < world.getHeight() - 55 &&
           world.regionPasses(
               x,
               y,
               3,
               1,
               [](Tile &tile) {
                   return isSolidBlock(tile.blockID) &&
                          tile.slope == Slope::none && !tile.actuated &&
                          !tile.hasAnyWire();
               }) &&
           world.regionPasses(x, y - 5, 3, 5, [](Tile &tile) {
               return tile.blockID == TileID::empty &&
                      (tile.liquid == Liquid::none ||
                       tile.liquid == Liquid::water) &&
                      !tile.hasAnyWire();
           });
}

Point selectTeleporterLocation(
    int maxBin,
    const LocationBins &locations,
    const std::vector<Point> &usedLocations,
    int minX,
    int maxX,
    Random &rnd,
    World &world)
{
    minX = std::max(55, minX);
    maxX = std::min(world.getWidth() - 58, maxX);
    for (int tries = 0; tries < 500; ++tries) {
        int binId = rnd.getInt(0, maxBin);
        auto binItr = locations.find(binId);
        if (binItr == locations.end() || binItr->second.empty()) {
            continue;
        }
        Point pt = rnd.select(binItr->second);
        if (!isLocationUsed(pt.x, pt.y, 75, usedLocations) &&
            isTeleporterLocation(pt.x, pt.y, minX, maxX, world)) {
            return pt;
        }
    }
    return {-1, -1};
}

void addJunctionBox(Tile &tile)
{
    if (tile.blockID == TileID::empty || isSolidBlock(tile.blockID)) {
        tile.blockID = TileID::junctionBox;
    }
}

#define APPLY_WIRE(COLOR)                                                      \
    if (tile.wire##COLOR) {                                                    \
        addJunctionBox(tile);                                                  \
    } else {                                                                   \
        tile.wire##COLOR = true;                                               \
    }                                                                          \
    break

void genTeleporters(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Wiring teleporters" << std::flush;
    bool showProgress = world.conf.teleporters * 460800.0 /
                            (world.getWidth() * world.getHeight()) >
                        1;
    if (!showProgress) {
        std::cout << '\n';
    }
    std::vector<Point> usedLocations{world.spawn};
    int maxBin =
        binLocation(world.getWidth(), world.getHeight(), world.getHeight());
    for (int remaining = world.conf.teleporters; remaining > 0; --remaining) {
        if (showProgress) {
            std::cout << '.' << std::flush;
        }

        int minX = 0;
        int maxX = world.getWidth();
        Point from = selectTeleporterLocation(
            maxBin,
            locations,
            usedLocations,
            minX,
            maxX,
            rnd,
            world);
        if (from.x == -1) {
            continue;
        }
        usedLocations.push_back(from);
        if (rnd.getStableUint(from.x, from.y) % 3 != 0) {
            minX = from.x - world.getHeight();
            maxX = from.x + world.getHeight();
        }
        Point to = selectTeleporterLocation(
            maxBin,
            locations,
            usedLocations,
            minX,
            maxX,
            rnd,
            world);
        if (to.x == -1) {
            continue;
        }
        usedLocations.push_back(to);

        std::vector<Point> path = findPath(
            from + Point{1, -2},
            to + Point{1, -2},
            [&world](Point pt) {
                int cost = 1;
                for (Point delta :
                     {Point{0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
                    Tile &tile = world.getTile(pt + delta);
                    if (tile.hasAnyWire()) {
                        cost += 13;
                    }
                    if (tile.actuator ||
                        wireAvoidTiles.contains(tile.blockID)) {
                        cost += 3;
                    }
                    if (wireAvoidWalls.contains(tile.wallID)) {
                        cost += 1;
                    }
                }
                for (Point delta : {Point{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}) {
                    Tile &tile = world.getTile(pt + delta);
                    if (tile.hasAnyWire()) {
                        cost += 2;
                    }
                }
                return cost;
            },
            [&world](Point pt) {
                return pt.x > 50 && pt.y > 50 && pt.x < world.getWidth() - 50 &&
                       pt.y < world.getHeight() - 50;
            });
        std::array crossings{0, 0, 0, 0};
        for (Point pt : path) {
            Tile &tile = world.getTile(pt);
            if (tile.wireRed) {
                crossings[0] += 1;
            }
            if (tile.wireBlue) {
                crossings[1] += 1;
            }
            if (tile.wireGreen) {
                crossings[2] += 1;
            }
            if (tile.wireYellow) {
                crossings[3] += 1;
            }
        }
        int wire = std::distance(
            crossings.begin(),
            std::min_element(crossings.begin(), crossings.end()));
        path.push_back(from + Point{1, -1});
        path.push_back(to + Point{1, -1});
        for (Point pt : path) {
            Tile &tile = world.getTile(pt);
            switch (wire) {
            case 0:
                APPLY_WIRE(Red);
            case 1:
                APPLY_WIRE(Blue);
            case 2:
                APPLY_WIRE(Green);
            case 3:
                APPLY_WIRE(Yellow);
            }
        }
        for (Point pt : {from, to}) {
            world.placeFramedTile(pt.x, pt.y - 1, TileID::teleporter);
            Tile &tile = world.getTile(pt + Point{1, -2});
            tile.blockID = TileID::pressurePlate;
            tile.frameY = 72;
        }
    }
    if (showProgress) {
        std::cout << '\n';
    }
}
