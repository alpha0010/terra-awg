#include "structures/StructureUtil.h"

#include "World.h"
#include "vendor/frozen/set.h"
#include <cmath>

int binLocation(int x, int y, int maxY)
{
    int factor = 128;
    x /= factor;
    y /= factor;
    maxY /= factor;
    return x * maxY + y;
}

bool isLocationUsed(
    int x,
    int y,
    int radius,
    const std::vector<Point> &usedLocations,
    int maxCount)
{
    int count = 0;
    for (auto [usedX, usedY] : usedLocations) {
        if (std::hypot(x - usedX, y - usedY) < radius) {
            ++count;
            if (count >= maxCount) {
                return true;
            }
        }
    }
    return false;
}

inline constexpr auto nonSolidTiles = frozen::make_set<int>({
    TileID::empty,
    TileID::alchemyTable,
    TileID::altar,
    TileID::amberTree,
    TileID::amethystTree,
    TileID::ashPlant,
    TileID::ashTree,
    TileID::ashVines,
    TileID::banner,
    TileID::bastStatue,
    TileID::bathtub,
    TileID::bed,
    TileID::bench,
    TileID::bewitchingTable,
    TileID::boneWelder,
    TileID::book,
    TileID::bookcase,
    TileID::borealBeam,
    TileID::bottle,
    TileID::boulder,
    TileID::bouncyBoulder,
    TileID::bubble,
    TileID::cactusPlant,
    TileID::campfire,
    TileID::candelabra,
    TileID::candle,
    TileID::catacomb,
    TileID::chain,
    TileID::chair,
    TileID::chandelier,
    TileID::chest,
    TileID::chestGroup2,
    TileID::clock,
    TileID::cobweb,
    TileID::coral,
    TileID::corruptPlant,
    TileID::corruptVines,
    TileID::crimsonPlant,
    TileID::crimsonVines,
    TileID::detonator,
    TileID::diamondTree,
    TileID::door,
    TileID::dresser,
    TileID::emeraldTree,
    TileID::explosives,
    TileID::fallenLog,
    TileID::flowerVines,
    TileID::gem,
    TileID::geyser,
    TileID::goldCoin,
    TileID::grassPlant,
    TileID::hellforge,
    TileID::herb,
    TileID::honeyDrip,
    TileID::junglePlant,
    TileID::jungleVines,
    TileID::lamp,
    TileID::lantern,
    TileID::largeJunglePlant,
    TileID::largePile,
    TileID::largePileGroup2,
    TileID::larva,
    TileID::lavaDrip,
    TileID::lifeCrystal,
    TileID::lifeCrystalBoulder,
    TileID::lihzahrdAltar,
    TileID::loom,
    TileID::manaCrystal,
    TileID::marbleColumn,
    TileID::minecartTrack,
    TileID::mossPlant,
    TileID::mushroomPlant,
    TileID::mushroomVines,
    TileID::orbHeart,
    TileID::painting2x3,
    TileID::painting3x2,
    TileID::painting3x3,
    TileID::painting6x4,
    TileID::palmTree,
    TileID::piano,
    TileID::platform,
    TileID::pot,
    TileID::pressurePlate,
    TileID::richMahoganyBeam,
    TileID::rollingCactus,
    TileID::rope,
    TileID::rubyTree,
    TileID::sakuraTree,
    TileID::sandDrip,
    TileID::sandstoneColumn,
    TileID::sapphireTree,
    TileID::sensor,
    TileID::sharpeningStation,
    TileID::silverCoin,
    TileID::sink,
    TileID::smallPile,
    TileID::stalactite,
    TileID::statue,
    TileID::sunflower,
    TileID::table,
    TileID::tableGroup2,
    TileID::tallGrassPlant,
    TileID::tallJunglePlant,
    TileID::TNTBarrel,
    TileID::toilet,
    TileID::tombstone,
    TileID::topazTree,
    TileID::torch,
    TileID::tree,
    TileID::vineRope,
    TileID::vines,
    TileID::waterCandle,
    TileID::waterDrip,
    TileID::woodenBeam,
    TileID::workBench,
    TileID::yellowWillowTree,
});

bool isSolidBlock(int tileId)
{
    return !nonSolidTiles.contains(tileId);
}

Point addPts(Point a, Point b)
{
    return {a.first + b.first, a.second + b.second};
}

Point subPts(Point a, Point b)
{
    return {a.first - b.first, a.second - b.second};
}

bool isInBounds(Point pt, World &world)
{
    return pt.first > 5 && pt.first < world.getWidth() - 5 && pt.second > 5 &&
           pt.second < world.getHeight() - 5;
}

Point scanWhileEmpty(Point from, Point delta, World &world)
{
    while (world.getTile(addPts(from, delta)).blockID == TileID::empty &&
           isInBounds(from, world)) {
        from = addPts(from, delta);
    }
    return from;
}

Point scanWhileNotSolid(Point from, Point delta, World &world)
{
    while (!isSolidBlock(world.getTile(addPts(from, delta)).blockID) &&
           isInBounds(from, world)) {
        from = addPts(from, delta);
    }
    return from;
}

void placeWire(Point from, Point to, Wire wire, World &world)
{
    auto enableWireAt = [wire, &world](Point pt) {
        Tile &tile = world.getTile(pt.first, pt.second);
        switch (wire) {
        case Wire::red:
            tile.wireRed = true;
            break;
        case Wire::blue:
            tile.wireBlue = true;
            break;
        case Wire::green:
            tile.wireGreen = true;
            break;
        case Wire::yellow:
            tile.wireYellow = true;
            break;
        }
    };
    enableWireAt(from);
    while (from != to) {
        if (from.first < to.first) {
            ++from.first;
        } else if (from.first > to.first) {
            --from.first;
        }
        enableWireAt(from);
        if (from.second < to.second) {
            ++from.second;
        } else if (from.second > to.second) {
            --from.second;
        }
        enableWireAt(from);
    }
}
