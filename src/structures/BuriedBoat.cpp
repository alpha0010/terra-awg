#include "structures/BuriedBoat.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/data/Boats.h"
#include <iostream>
#include <set>

typedef std::pair<int, int> Point;

Point selectBoatLocation(int width, int height, Random &rnd, World &world)
{
    int xMin = world.snowCenter - 0.06 * world.getWidth() - width;
    int xMax = world.snowCenter + 0.06 * world.getWidth();
    int yMax =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3 - height;
    std::set<int> avoidBlocks{
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
    };
    while (true) {
        int x = rnd.getInt(xMin, xMax);
        int y = rnd.getInt(world.getCavernLevel(), yMax);
        if (world.regionPasses(x, y, width, height, [&avoidBlocks](Tile &tile) {
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
    std::set<int> clearableTiles{
        TileID::borealWood,
        TileID::richMahogany,
        TileID::richMahoganyBeam,
        TileID::rope};
    std::set<Point> chests;
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
                !chests.contains({i - 1, j}) && !chests.contains({i, j - 1}) &&
                !chests.contains({i - 1, j - 1})) {
                chests.emplace(i, j);
            }
            if (boatTile.wallID == WallID::empty) {
                boatTile.wallID = tile.wallID;
            }
            tile = boatTile;
            tile.guarded = true;
        }
    }
    for (auto [i, j] : chests) {
        Chest &chest = world.placeChest(x + i, y + j, Variant::frozen);
        fillCavernFrozenChest(chest, rnd, world);
    }
}
