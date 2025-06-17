#include "structures/BuriedBoat.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/data/Boats.h"
#include "vendor/frozen/set.h"
#include <iostream>

typedef std::pair<int, int> Point;

Point selectBoatLocation(int width, int height, Random &rnd, World &world)
{
    int xMin = world.conf.patches
                   ? 350
                   : world.snowCenter - 0.06 * world.getWidth() - width;
    int xMax = world.conf.patches ? world.getWidth() - width - 350
                                  : world.snowCenter + 0.06 * world.getWidth();
    int yMax =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3 - height;
    constexpr auto avoidBlocks = frozen::make_set<int>({
        TileID::aetherium,
        TileID::blueBrick,
        TileID::corruptIce,
        TileID::crimsonIce,
        TileID::dirt,
        TileID::granite,
        TileID::greenBrick,
        TileID::marble,
        TileID::mushroomGrass,
        TileID::pinkBrick,
        TileID::stone,
    });
    int biomeScan = std::max(width, height) / 2;
    while (true) {
        int x = rnd.getInt(xMin, xMax);
        int y = rnd.getInt(world.getCavernLevel(), yMax);
        if ((!world.conf.patches || isInBiome(
                                        x + biomeScan,
                                        y + biomeScan,
                                        biomeScan,
                                        Biome::snow,
                                        world)) &&
            world.regionPasses(x, y, width, height, [&avoidBlocks](Tile &tile) {
                return !avoidBlocks.contains(tile.blockID);
            })) {
            return {x, y};
        }
    }
}

void genBuriedBoat(Random &rnd, World &world)
{
    std::cout << "Misplacing explorers\n";
    rnd.shuffleNoise();
    TileBuffer boat = Data::getBoat(Data::Boat::frozen, world.getFramedTiles());
    auto [x, y] =
        selectBoatLocation(boat.getWidth(), boat.getHeight(), rnd, world);
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::borealWood,
         TileID::richMahogany,
         TileID::richMahoganyBeam,
         TileID::rope});
    for (int i = 0; i < boat.getWidth(); ++i) {
        for (int j = 0; j < boat.getHeight(); ++j) {
            Tile &boatTile = boat.getTile(i, j);
            if (boatTile.blockID == TileID::empty &&
                boatTile.wallID == WallID::empty) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (std::abs(rnd.getFineNoise(x + i, y + j)) < 0.11) {
                if (tile.blockID != TileID::empty &&
                    clearableTiles.contains(boatTile.blockID)) {
                    continue;
                }
                if (boatTile.blockID == TileID::empty) {
                    boatTile.blockID = tile.blockID;
                } else if (tile.wallID != WallID::empty) {
                    boatTile.wallID = tile.wallID;
                    boatTile.wallPaint = tile.wallPaint;
                }
            }
            if (boatTile.blockID == TileID::chest &&
                boatTile.frameX % 36 == 0 && boatTile.frameY == 0) {
                fillCavernFrozenChest(
                    world.registerStorage(x + i, y + j),
                    rnd,
                    world);
            }
            if (boatTile.wallID == WallID::empty) {
                boatTile.wallID = tile.wallID;
            }
            tile = boatTile;
            tile.guarded = true;
        }
    }
}
