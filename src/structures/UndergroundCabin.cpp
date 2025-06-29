#include "structures/UndergroundCabin.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Statues.h"
#include "structures/StructureUtil.h"
#include "structures/data/DynCabin.h"
#include "vendor/frozen/set.h"
#include <algorithm>

inline constexpr auto cabinClearTiles = frozen::make_set<int>({
    TileID::empty,         TileID::dirt,        TileID::stone,
    TileID::clay,          TileID::sand,        TileID::mud,
    TileID::copperOre,     TileID::tinOre,      TileID::ironOre,
    TileID::leadOre,       TileID::silverOre,   TileID::tungstenOre,
    TileID::goldOre,       TileID::platinumOre, TileID::cobaltOre,
    TileID::palladiumOre,  TileID::mythrilOre,  TileID::orichalcumOre,
    TileID::adamantiteOre, TileID::titaniumOre, TileID::chlorophyteOre,
});

bool canPlaceOnCabinGround(int x, int y, int width, int height, World &world)
{
    return world.regionPasses(x, y, width, height, [](Tile &tile) {
        return tile.blockID == TileID::empty;
    }) && world.regionPasses(x, y + height, width, 1, [](Tile &tile) {
        return tile.guarded && isSolidBlock(tile.blockID);
    });
}

void applySupportBeam(int x, int y, World &world)
{
    if (world.getTile(x, y).blockID != TileID::empty) {
        return;
    }
    int supFloor = scanWhileEmpty({x, y}, {0, 1}, world).second;
    ++supFloor;
    if (supFloor - y < 2 || supFloor - y > 24 ||
        !cabinClearTiles.contains(world.getTile(x, supFloor).blockID)) {
        return;
    }
    for (int beamY = y; beamY < supFloor; ++beamY) {
        world.getTile(x, beamY).blockID = TileID::woodenBeam;
    }
}

void addCabinPainting(std::vector<Point> &locations, Random &rnd, World &world)
{
    Painting painting = rnd.pool(
        {Painting::americanExplosive,
         Painting::secrets,
         Painting::strangeDeadFellows,
         Painting::sufficientlyAdvanced,
         Painting::bifrost,
         Painting::bioluminescence,
         Painting::findingGold,
         Painting::forestTroll,
         Painting::aHorribleNightForAlchemy,
         Painting::catSword,
         Painting::crownoDevoursHisLunch,
         Painting::fairyGuides,
         Painting::fatherOfSomeone,
         Painting::guidePicasso,
         Painting::nurseLisa,
         Painting::outcast,
         Painting::rareEnchantment,
         Painting::terrarianGothic});
    auto [pWidth, pHeight] = world.getPaintingDims(painting);
    for (auto [x, y] : locations) {
        if (world.regionPasses(
                x - 1,
                y,
                pWidth + 2,
                pHeight + 1,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(x, y, pWidth, pHeight, [](Tile &tile) {
                return tile.wallID != WallID::empty;
            })) {
            world.placePainting(x, y, painting);
            break;
        }
    }
}

Point addCabinStatue(std::vector<Point> &locations, int statue, World &world)
{
    for (auto [x, y] : locations) {
        if (!world.regionPasses(
                x - 1,
                y,
                4,
                3,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(x, y + 3, 2, 1, [](Tile &tile) {
                return tile.guarded && isSolidBlock(tile.blockID);
            })) {
            continue;
        }
        placeStatue(x, y, statue, world);
        return {x, y};
    }
    return {-1, -1};
}

void addCabinDebris(std::vector<Point> &locations, World &world)
{
    for (auto [x, y] : locations) {
        if (!world.regionPasses(
                x,
                y,
                3,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(x, y + 2, 3, 1, [](Tile &tile) {
                return isSolidBlock(tile.blockID);
            })) {
            continue;
        }
        world.placeFramedTile(x, y, TileID::largePile, Variant::furniture);
        break;
    }
    for (auto [x, y] : locations) {
        if (!world.regionPasses(
                x,
                y,
                2,
                1,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(x, y + 1, 2, 1, [](Tile &tile) {
                return isSolidBlock(tile.blockID);
            })) {
            continue;
        }
        world.placeFramedTile(x, y, TileID::smallPile, Variant::furniture);
        break;
    }
    for (auto [x, y] : locations) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::empty && fnv1a32pt(x, y) % 19 == 0) {
            tile.blockID = TileID::cobweb;
        }
    }
}

void addCabinBarrel(std::vector<Point> &locations, Random &rnd, World &world)
{
    for (auto [x, y] : locations) {
        if (canPlaceOnCabinGround(x, y, 2, 2, world)) {
            fillBarrel(world.placeChest(x, y, Variant::barrel), rnd);
            break;
        }
    }
}

void addCabinStation(std::vector<Point> &locations, Random &rnd, World &world)
{
    for (auto [x, y] : locations) {
        if (canPlaceOnCabinGround(x, y, 3, 2, world)) {
            world.placeFramedTile(
                x,
                y,
                rnd.select({TileID::loom, TileID::sharpeningStation}));
            break;
        }
    }
}

void maybePlaceCabinForChest(int x, int y, Random &rnd, World &world)
{
    if (rnd.getDouble(0, 1) > 0.4) {
        return;
    }
    TileBuffer cabin = Data::getCabin(
        rnd.select(Data::cabins),
        rnd.getInt(16, 26),
        world.getFramedTiles());
    int chestX = x;
    int chestY = y - 2;
    x -= cabin.getWidth() / 2 + rnd.getInt(-5, 4);
    y -= cabin.getHeight() - 1;
    if (!world.regionPasses(
            x,
            y,
            cabin.getWidth(),
            cabin.getHeight(),
            [](Tile &tile) {
                return !tile.guarded && cabinClearTiles.contains(tile.blockID);
            })) {
        return;
    }
    std::vector<Point> locations;
    for (int i = 0; i < cabin.getWidth(); ++i) {
        int doorAt = -1;
        for (int j = 0; j < cabin.getHeight(); ++j) {
            Tile &cabinTile = cabin.getTile(i, j);
            if (cabinTile.blockID == TileID::cloud) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (std::abs(rnd.getFineNoise(x + 3 * i, y + 3 * j)) > 0.34) {
                if (tile.blockID == TileID::empty &&
                    fnv1a32pt(x + i, y + j) % 5 != 0) {
                    tile = {};
                } else {
                    tile.liquid = Liquid::none;
                    if (isSolidBlock(cabinTile.blockID)) {
                        tile.guarded = true;
                    }
                }
                continue;
            }
            if (cabinTile.blockID == TileID::door) {
                doorAt = j - (cabinTile.frameY % 54) / 18;
            } else {
                tile = cabinTile;
                tile.guarded = true;
                locations.emplace_back(x + i, y + j);
            }
        }
        if (doorAt != -1) {
            if (world.getTile(x + i, y + doorAt - 1).blockID != TileID::empty &&
                world.getTile(x + i, y + doorAt + 3).blockID != TileID::empty) {
                for (int j = doorAt; j < doorAt + 3; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    tile = cabin.getTile(i, j);
                    tile.guarded = true;
                }
            }
        }
        if ((i + y) % 5 == 0) {
            applySupportBeam(x + i, y + cabin.getHeight(), world);
        }
    }
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            world.getTile(chestX + i, chestY + j).blockID = TileID::chest;
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    addCabinPainting(locations, rnd, world);
    addCabinStatue(locations, rnd.select(StatueVariants::deco), world);
    Point trap =
        addCabinStatue(locations, rnd.select(StatueVariants::enemy), world);
    switch (rnd.getInt(0, 5)) {
    case 0:
        addCabinStation(locations, rnd, world);
        [[fallthrough]];
    case 1:
        addCabinBarrel(locations, rnd, world);
        break;
    case 2:
        addCabinBarrel(locations, rnd, world);
        [[fallthrough]];
    case 3:
        addCabinStation(locations, rnd, world);
        break;
    }
    addCabinDebris(locations, world);
    if (trap.first != -1 && rnd.getDouble(0, 1) < 0.2 * world.conf.traps) {
        world.queuedTraps.emplace_back(
            [chestX, chestY, trap](Random &, World &world) {
                world.placeFramedTile(
                    chestX,
                    chestY,
                    TileID::chestGroup2,
                    Variant::deadMans);
                placeWire(trap, {chestX, chestY}, Wire::red, world);
            });
    }
}
