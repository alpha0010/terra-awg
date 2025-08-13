#include "structures/StructureUtil.h"

#include "World.h"
#include "ids/Paint.h"
#include "vendor/frozen/set.h"
#include <cmath>

int makeCongruent(int val, int mod)
{
    return mod * (val / mod);
}

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
    TileID::chimney,
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
    TileID::fireplace,
    TileID::flowerVines,
    TileID::gem,
    TileID::geyser,
    TileID::goldCoin,
    TileID::graniteColumn,
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
    TileID::weatherVane,
    TileID::woodenBeam,
    TileID::workBench,
    TileID::yellowWillowTree,
});

bool isSolidBlock(int tileId)
{
    return !nonSolidTiles.contains(tileId);
}

bool isInBounds(Point pt, World &world)
{
    return pt.x > 5 && pt.x < world.getWidth() - 5 && pt.y > 5 &&
           pt.y < world.getHeight() - 5;
}

Point scanWhileEmpty(Point from, Point delta, World &world)
{
    while (world.getTile(from + delta).blockID == TileID::empty &&
           isInBounds(from, world)) {
        from += delta;
    }
    return from;
}

Point scanWhileNotSolid(Point from, Point delta, World &world)
{
    while (!isSolidBlock(world.getTile(from + delta).blockID) &&
           isInBounds(from, world)) {
        from += delta;
    }
    return from;
}

inline std::array paintRainbow{
    Paint::red,     Paint::orange, Paint::yellow,  Paint::lime,   Paint::green,
    Paint::teal,    Paint::cyan,   Paint::skyBlue, Paint::blue,   Paint::purple,
    Paint::violet,  Paint::pink,   Paint::violet,  Paint::purple, Paint::blue,
    Paint::skyBlue, Paint::cyan,   Paint::teal,    Paint::green,  Paint::lime,
    Paint::yellow,  Paint::orange, Paint::red};

int getRainbowPaint(int x, int y)
{
    return paintRainbow[(x + y) % paintRainbow.size()];
}

inline std::array paintDeepRainbow{
    Paint::deepRed,    Paint::deepOrange, Paint::deepYellow, Paint::deepLime,
    Paint::deepGreen,  Paint::deepTeal,   Paint::deepCyan,   Paint::deepSkyBlue,
    Paint::deepBlue,   Paint::deepPurple, Paint::deepViolet, Paint::deepPink,
    Paint::deepViolet, Paint::deepPurple, Paint::deepBlue,   Paint::deepSkyBlue,
    Paint::deepCyan,   Paint::deepTeal,   Paint::deepGreen,  Paint::deepLime,
    Paint::deepYellow, Paint::deepOrange, Paint::deepRed};

int getDeepRainbowPaint(int x, int y)
{
    return paintDeepRainbow[(x + y) % paintDeepRainbow.size()];
}

void placeWire(Point from, Point to, Wire wire, World &world)
{
    auto enableWireAt = [wire, &world](Point pt) {
        Tile &tile = world.getTile(pt);
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
        if (from.x < to.x) {
            ++from.x;
        } else if (from.x > to.x) {
            --from.x;
        }
        enableWireAt(from);
        if (from.y < to.y) {
            ++from.y;
        } else if (from.y > to.y) {
            --from.y;
        }
        enableWireAt(from);
    }
}
