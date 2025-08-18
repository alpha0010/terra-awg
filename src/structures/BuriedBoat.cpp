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

Point selectBoatLocation(int width, int height, Random &rnd, World &world)
{
    int minX = world.conf.biomes == BiomeLayout::columns
                   ? world.snowCenter -
                         world.conf.snowSize * 0.06 * world.getWidth() - width
                   : 350;
    int maxX =
        world.conf.biomes == BiomeLayout::columns
            ? world.snowCenter + world.conf.snowSize * 0.06 * world.getWidth()
            : world.getWidth() - width - 350;
    int minY = world.conf.dontDigUp ? world.getUndergroundLevel() + height
                                    : world.getCavernLevel();
    int maxY =
        world.conf.dontDigUp
            ? world.getCavernLevel()
            : (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3 -
                  height;
    constexpr auto avoidBlocks = frozen::make_set<int>({
        TileID::aetherium,
        TileID::blueBrick,
        TileID::corruptIce,
        TileID::crimsonIce,
        TileID::dirt,
        TileID::granite,
        TileID::greenBrick,
        TileID::hive,
        TileID::lihzahrdBrick,
        TileID::marble,
        TileID::mushroomGrass,
        TileID::pinkBrick,
        TileID::stone,
    });
    int biomeScan = std::max(width, height) / 2;
    for (int tries = 0; tries < 8000; ++tries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        int maxEmpty = 0.7 * width * height;
        if ((world.conf.biomes == BiomeLayout::columns || isInBiome(
                                                              x + biomeScan,
                                                              y + biomeScan,
                                                              biomeScan,
                                                              Biome::snow,
                                                              world)) &&
            world.regionPasses(
                x,
                y,
                width,
                height,
                [&avoidBlocks, &maxEmpty](Tile &tile) {
                    if (tile.blockID == TileID::empty) {
                        --maxEmpty;
                    }
                    return !avoidBlocks.contains(tile.blockID) && maxEmpty > 0;
                })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genBuriedBoat(Random &rnd, World &world)
{
    std::cout << "Misplacing explorers\n";
    rnd.shuffleNoise();
    TileBuffer boat = Data::getBoat(Data::Boat::frozen, world.getFramedTiles());
    auto [x, y] =
        selectBoatLocation(boat.getWidth(), boat.getHeight(), rnd, world);
    if (x == -1) {
        return;
    }
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
                fillFrozenChest(
                    world.registerStorage(x + i, y + j),
                    Depth::cavern,
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
