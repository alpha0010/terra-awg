#include "structures/StarterHome.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Furniture.h"
#include "structures/data/Homes.h"
#include "vendor/frozen/map.h"
#include <iostream>
#include <map>

void placeHomeAt(
    int x,
    int y,
    Data::Variant theme,
    TileBuffer &home,
    Random &rnd,
    World &world)
{
    std::map<int, int> themeTiles;
    std::map<int, int> themeWalls;
    switch (theme) {
    case Data::Variant::ashWood:
        themeTiles.insert({{TileID::wood, TileID::ashWood}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::ashWood},
             {WallID::Safe::planked,
              rnd.select({WallID::Safe::obsidianBrick, WallID::Safe::planked})},
             {WallID::Safe::stone,
              rnd.select({WallID::Safe::ember, WallID::Safe::wornStone})}});
        break;
    case Data::Variant::boreal:
        themeTiles.insert(
            {{TileID::wood, TileID::borealWood},
             {TileID::woodenBeam, TileID::borealBeam},
             {TileID::grayBrick,
              rnd.select({TileID::snowBrick, TileID::iceBrick})}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::borealWood},
             {WallID::Safe::stone,
              rnd.select({WallID::Safe::snow, WallID::Safe::ice})}});
        break;
    case Data::Variant::forest:
        themeWalls.insert(
            {{WallID::Safe::stone,
              rnd.select({WallID::Safe::mudstoneBrick, WallID::Safe::stone})}});
        break;
    case Data::Variant::livingWood:
        themeTiles.insert({{TileID::wood, TileID::livingWood}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::livingWood},
             {WallID::Safe::planked,
              rnd.select({WallID::Safe::livingLeaf, WallID::Safe::planked})},
             {WallID::Safe::stone,
              rnd.select({WallID::Safe::mudstoneBrick, WallID::Safe::stone})}});
        break;
    case Data::Variant::mahogany:
        themeTiles.insert(
            {{TileID::wood, TileID::richMahogany},
             {TileID::woodenBeam, TileID::richMahoganyBeam},
             {TileID::grayBrick,
              rnd.select({TileID::iridescentBrick, TileID::mudstoneBrick})}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::richMahogany},
             {WallID::Safe::planked,
              rnd.select(
                  {WallID::Safe::whiteDynasty, WallID::Safe::blueDynasty})},
             {WallID::Safe::stone,
              rnd.select(
                  {WallID::Safe::bamboo,
                   WallID::Safe::largeBamboo,
                   WallID::Safe::richMahoganyFence})}});
        break;
    case Data::Variant::marble:
        themeTiles.insert(
            {{TileID::wood, TileID::smoothMarble},
             {TileID::woodenBeam, TileID::marbleColumn},
             {TileID::grayBrick,
              rnd.select({TileID::grayBrick, TileID::pearlstoneBrick})}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::smoothMarble},
             {WallID::Safe::planked,
              rnd.select(
                  {WallID::Safe::fancyGrayWallpaper,
                   WallID::Safe::platinumBrick})},
             {WallID::Safe::stone,
              rnd.select(
                  {WallID::Safe::marble, WallID::Safe::stalactiteStone})}});
        break;
    case Data::Variant::palm:
        themeTiles.insert(
            {{TileID::wood, TileID::palmWood},
             {TileID::woodenBeam, TileID::sandstoneColumn},
             {TileID::grayBrick,
              rnd.select({TileID::sandstoneBrick, TileID::tinBrick})}});
        themeWalls.insert(
            {{WallID::Safe::wood, WallID::Safe::palmWood},
             {WallID::Safe::planked,
              rnd.select({WallID::Safe::palmWoodFence, WallID::Safe::planked})},
             {WallID::Safe::stone,
              rnd.select(
                  {WallID::Safe::hardenedSand, WallID::Safe::sandstone})}});
        break;
    default:
        break;
    }
    if (!themeTiles.contains(TileID::grayBrick)) {
        themeTiles[TileID::grayBrick] =
            rnd.select({TileID::grayBrick, TileID::redBrick, TileID::tinBrick});
    }
    constexpr auto brickToWall = frozen::make_map<int, int>({
        {TileID::grayBrick, WallID::Safe::grayBrick},
        {TileID::iceBrick, WallID::Safe::iceBrick},
        {TileID::iridescentBrick, WallID::Safe::iridescentBrick},
        {TileID::mudstoneBrick, WallID::Safe::mudstoneBrick},
        {TileID::pearlstoneBrick, WallID::Safe::pearlstoneBrick},
        {TileID::redBrick, WallID::Safe::redBrick},
        {TileID::sandstoneBrick, WallID::Safe::sandstoneBrick},
        {TileID::snowBrick, WallID::Safe::snowBrick},
        {TileID::tinBrick, WallID::Safe::tinBrick},
    });
    themeWalls[WallID::Safe::grayBrick] =
        brickToWall.at(themeTiles[TileID::grayBrick]);
    for (int i = 0; i < home.getWidth(); ++i) {
        for (int j = 0; j < home.getHeight(); ++j) {
            Tile &homeTile = home.getTile(i, j);
            if (homeTile.blockID == TileID::cloud ||
                (homeTile.blockID == TileID::empty &&
                 homeTile.wallID == WallID::empty)) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID != TileID::empty &&
                (tile.blockID != TileID::thinIce ||
                 world.getTile(x + i, y + j + 1).blockID != TileID::empty) &&
                j > home.getHeight() / 2) {
                continue;
            }
            if (homeTile.blockID == TileID::chest &&
                homeTile.frameX % 36 == 0 && homeTile.frameY == 0) {
                fillStarterChest(
                    world.conf.equipment,
                    world.registerStorage(x + i, y + j),
                    rnd);
            }
            convertFurniture(homeTile, theme);
            auto tileItr = themeTiles.find(homeTile.blockID);
            if (tileItr != themeTiles.end()) {
                homeTile.blockID = tileItr->second;
            }
            auto wallItr = themeWalls.find(homeTile.wallID);
            if (wallItr != themeWalls.end()) {
                homeTile.wallID = wallItr->second;
            }
            if (tile.blockID == TileID::thinIce &&
                homeTile.blockID == TileID::empty) {
                homeTile.blockID = TileID::thinIce;
            }
            homeTile.liquid = tile.liquid;
            tile = homeTile;
            tile.guarded = true;
        }
    }
}

int realSurfaceAt(int x, World &world)
{
    int minY = world.getSurfaceLevel(x) - 75;
    while (world.getTile(x, minY).blockID != TileID::empty) {
        --minY;
    }
    return scanWhileEmpty({x, minY}, {0, 1}, world).second + 1;
}

void genStarterHome(Random &rnd, World &world)
{
    std::cout << "Purchasing property\n";
    std::map<int, int> tileCounts;
    int x = world.getWidth() / 2;
    int y = realSurfaceAt(x, world);
    for (int j = 0; j < 6; ++j) {
        tileCounts[world.getTile(x, y + j).blockID] += 1;
    }
    Data::Variant theme = Data::Variant::forest;
    if (tileCounts[TileID::ash] > 2) {
        theme = Data::Variant::ashWood;
    } else if (tileCounts[TileID::snow] > 2) {
        theme = Data::Variant::boreal;
    } else if (
        tileCounts[TileID::jungleGrass] > 0 && tileCounts[TileID::mud] > 1) {
        theme = Data::Variant::mahogany;
    } else if (tileCounts[TileID::marble] > 2) {
        theme = Data::Variant::marble;
    } else if (tileCounts[TileID::sand] > 2) {
        theme = Data::Variant::palm;
    } else if (!world.regionPasses(x - 100, y, 200, 1, [](Tile &tile) {
                   return tile.blockID != TileID::livingWood;
               })) {
        theme = Data::Variant::livingWood;
    }
    TileBuffer home =
        Data::getHome(rnd.select(Data::homes), world.getFramedTiles());
    x -= home.getWidth() / 2;
    for (int i = 7; i < home.getWidth() - 7; ++i) {
        y = std::min(realSurfaceAt(x + i, world), y);
    }
    world.spawnY = y - 1;
    for (int j = 0; j < home.getHeight(); ++j) {
        --y;
        if (home.getTile(0, j).blockPaint == Paint::red) {
            break;
        }
    }
    placeHomeAt(x, y, theme, home, rnd, world);
}
