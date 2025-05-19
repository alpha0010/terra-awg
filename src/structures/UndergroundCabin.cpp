#include "structures/UndergroundCabin.h"

#include "Random.h"
#include "World.h"
#include "structures/StructureUtil.h"
#include "structures/data/DynCabin.h"
#include <set>

inline const std::set<int> cabinClearTiles{
    TileID::empty,
    TileID::dirt,
    TileID::stone,
    TileID::clay,
    TileID::sand,
    TileID::mud,
    TileID::copperOre,
    TileID::tinOre,
    TileID::ironOre,
    TileID::leadOre,
    TileID::silverOre,
    TileID::tungstenOre,
    TileID::goldOre,
    TileID::platinumOre,
};

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

void maybePlaceCabinForChest(int x, int y, Random &rnd, World &world)
{
    if (rnd.getDouble(0, 1) > 0.4) {
        return;
    }
    TileBuffer cabin = Data::getCabin(
        rnd.select(Data::cabins),
        rnd.getInt(16, 24),
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
    for (int i = 0; i < cabin.getWidth(); ++i) {
        int doorAt = -1;
        for (int j = 0; j < cabin.getHeight(); ++j) {
            if (std::abs(rnd.getFineNoise(x + 3 * i, y + 3 * j)) > 0.34) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            Tile &cabinTile = cabin.getTile(i, j);
            if (cabinTile.blockID == TileID::door) {
                doorAt = j - (cabinTile.frameY % 54) / 18;
            } else {
                tile = cabinTile;
                tile.guarded = true;
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
}
