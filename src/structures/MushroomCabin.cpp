#include "structures/MushroomCabin.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Statues.h"
#include "structures/StructureUtil.h"
#include "structures/data/Furniture.h"
#include "structures/data/Mushrooms.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

Point selectCabinLocation(
    Point center,
    int cabinWidth,
    Random &rnd,
    World &world)
{
    int scanX = 0.02 * world.getWidth();
    int scanY = scanX / 2;
    constexpr auto allowedTiles = frozen::make_set<int>(
        {TileID::empty,
         TileID::mud,
         TileID::mushroomGrass,
         TileID::silt,
         TileID::slime});
    for (int tries = 0; tries < 1000; ++tries) {
        int x = rnd.getInt(center.x - scanX - cabinWidth, center.x + scanX);
        int y = rnd.getInt(center.y - scanY, center.y + scanY);
        if (world.regionPasses(x, y, cabinWidth, 18, [](Tile &tile) {
                return tile.blockID == TileID::empty &&
                       (tile.wallID == WallID::empty ||
                        tile.wallID == WallID::Unsafe::mushroom);
            })) {
            std::vector<int> surface;
            for (int i = 0; i < cabinWidth; ++i) {
                surface.push_back(scanWhileEmpty({x + i, y}, {0, 1}, world).y);
            }
            std::sort(surface.begin(), surface.end());
            y = surface[0.75 * surface.size()];
            if (world.regionPasses(
                    x - 5,
                    y - 18,
                    cabinWidth + 10,
                    20,
                    [&allowedTiles](Tile &tile) {
                        return !tile.guarded &&
                               allowedTiles.contains(tile.blockID);
                    })) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

int placeMushroomStruct(
    int x,
    int y,
    int mushroomId,
    int paint,
    Random &rnd,
    World &world)
{
    TileBuffer data = Data::getMushroom(mushroomId, world.getFramedTiles());
    bool mirror = rnd.getBool();
    for (int i = 0; i < data.getWidth(); ++i) {
        int dataI = mirror ? data.getWidth() - 1 - i : i;
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &dataTile = data.getTile(dataI, j);
            if (dataTile.blockID == TileID::empty) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j + 1 - data.getHeight());
            tile.blockID =
                std::abs(rnd.getFineNoise(x + 3 * i, y + 3 * j)) > 0.3
                    ? TileID::slime
                    : TileID::mud;
            tile.blockPaint =
                tile.blockID == TileID::slime ? paint : Paint::none;
            tile.wallID = WallID::Unsafe::mushroom;
            tile.liquid = Liquid::none;
        }
    }
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &tile = world.getTile(x + i, y + j + 1 - data.getHeight());
            if (!world.isExposed(x + i, y + j + 1 - data.getHeight())) {
                tile.guarded = true;
            } else if (tile.blockID == TileID::mud) {
                tile.blockID = TileID::mushroomGrass;
                tile.blockPaint = paint;
            }
        }
    }
    return data.getWidth() / 2;
}

bool canPlaceCabinFurniture(int x, int y, TileBuffer &data, World &world)
{
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            switch (data.getTile(i, j).blockID) {
            case TileID::empty:
                if (isSolidBlock(tile.blockID)) {
                    return false;
                }
                break;
            case TileID::cloud:
                if (!isSolidBlock(tile.blockID) || tile.actuated) {
                    return false;
                }
                break;
            default:
                if (tile.blockID != TileID::empty) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}

void placeMushroomFurniture(
    std::vector<Point> &&locations,
    Random &rnd,
    World &world)
{
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    std::vector<int> furniture{
        Data::furnitureLayouts.begin(),
        Data::furnitureLayouts.end()};
    std::shuffle(furniture.begin(), furniture.end(), rnd.getPRNG());
    auto furnitureItr = furniture.begin();
    double numPlacements = locations.size() / 50.0;
    int tries = 0;
    TileBuffer data;
    while (numPlacements > 0) {
        for (auto [x, y] : locations) {
            if (tries > 30) {
                tries = 0;
                numPlacements -= 0.1;
            }
            if (tries == 0) {
                data = getFurniture(
                    *furnitureItr,
                    Data::Variant::mushroom,
                    world.getFramedTiles());
                ++furnitureItr;
                if (furnitureItr == furniture.end()) {
                    furnitureItr = furniture.begin();
                }
            }
            ++tries;
            if (!canPlaceCabinFurniture(x, y, data, world)) {
                continue;
            }
            for (auto [chestX, chestY] :
                 world.placeBuffer(x, y, data, Blend::blockOnly)) {
                fillDresser(world.registerStorage(chestX, chestY), rnd);
            }
            tries = 0;
            --numPlacements;
            if (numPlacements < 0) {
                break;
            }
        }
    }
}

void genMushroomCabin(Random &rnd, World &world)
{
    std::cout << "Shaping mushrooms\n";
    std::vector<int> structIds{Data::mushrooms.begin(), Data::mushrooms.end()};
    std::shuffle(structIds.begin(), structIds.end(), rnd.getPRNG());
    auto mushroomItr = structIds.begin();
    for (Point center : world.mushroomCenter) {
        int cabinWidth = rnd.getInt(20, 30);
        auto [x, y] = selectCabinLocation(center, cabinWidth, rnd, world);
        if (x == -1) {
            continue;
        }
        for (int i = 0; i < cabinWidth; ++i) {
            for (int j = -10; j < 1; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.wallID != WallID::empty ||
                    rnd.getFineNoise(x + i, y + j) > 0.5) {
                    tile.wallID = WallID::Unsafe::mud;
                }
            }
        }
        bool isHallow =
            !world.regionPasses(x, y, cabinWidth, 10, [](Tile &tile) {
                return tile.blockPaint != Paint::pink;
            });
        for (int i = -10; i < cabinWidth - 8; i += 2) {
            i += placeMushroomStruct(
                x + i,
                y,
                *mushroomItr,
                isHallow ? Paint::pink : Paint::none,
                rnd,
                world);
            ++mushroomItr;
            if (mushroomItr == structIds.end()) {
                mushroomItr = structIds.begin();
            }
        }
        std::vector<Point> locations;
        for (int i = 0; i < cabinWidth; ++i) {
            for (int j = -8; j < 1; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                locations.emplace_back(x + i, y + j);
                if (i == 0 || i == cabinWidth - 1 || j == -8 || j == 0) {
                    if (tile.blockID == TileID::empty ||
                        (j > -5 && tile.blockID == TileID::mud)) {
                        tile.blockID = TileID::glowingMushroom;
                    }
                } else {
                    tile.blockID = TileID::empty;
                }
                tile.liquid = Liquid::none;
            }
        }
        world.placeFramedTile(x, y - 3, TileID::door, Variant::mushroom);
        world.placeFramedTile(
            x + cabinWidth - 1,
            y - 3,
            TileID::door,
            Variant::mushroom);
        Chest &chest = world.placeChest(
            x + rnd.getDouble(0.25, 0.75) * cabinWidth,
            y - 2,
            Variant::mushroom);
        fillMushroomChest(
            chest,
            getChestDepth(chest.x, chest.y, world),
            rnd,
            world);
        int statueX = x + rnd.getDouble(0.1, 0.9) * cabinWidth;
        if (world.regionPasses(statueX, y - 3, 2, 3, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            placeStatue(
                statueX,
                y - 3,
                rnd.pool(StatueVariants::utility),
                world);
        }
        statueX = x + rnd.getDouble(0.1, 0.9) * cabinWidth;
        if (world.regionPasses(statueX, y - 3, 2, 3, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            placeStatue(
                statueX,
                y - 3,
                rnd.select(StatueVariants::enemy),
                world);
        }
        statueX = x + rnd.getDouble(0.1, 0.9) * cabinWidth;
        if (world.regionPasses(statueX, y - 3, 2, 3, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            placeStatue(statueX, y - 3, Statue::mushroom, world);
        }
        placeMushroomFurniture(std::move(locations), rnd, world);
    }
}
