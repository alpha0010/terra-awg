#include "biomes/doubleTrouble/ResourceSwap.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "vendor/frozen/map.h"
#include <iostream>

void swapResources(Random &rnd, World &world)
{
    std::cout << "Shuffling ores\n";
    rnd.restoreShuffleState();
    constexpr auto oreSwap = frozen::make_map<int, int>({
        {TileID::copperOre, TileID::tinOre},
        {TileID::tinOre, TileID::copperOre},
        {TileID::ironOre, TileID::leadOre},
        {TileID::leadOre, TileID::ironOre},
        {TileID::silverOre, TileID::tungstenOre},
        {TileID::tungstenOre, TileID::silverOre},
        {TileID::goldOre, TileID::platinumOre},
        {TileID::platinumOre, TileID::goldOre},
        {TileID::cobaltOre, TileID::palladiumOre},
        {TileID::palladiumOre, TileID::cobaltOre},
        {TileID::mythrilOre, TileID::orichalcumOre},
        {TileID::orichalcumOre, TileID::mythrilOre},
        {TileID::adamantiteOre, TileID::titaniumOre},
        {TileID::titaniumOre, TileID::adamantiteOre},
    });
    constexpr auto blockSwap = frozen::make_map<int, int>({
        {TileID::marble, TileID::granite},
        {TileID::granite, TileID::marble},
        {TileID::smoothMarble, TileID::smoothGranite},
        {TileID::smoothGranite, TileID::smoothMarble},
    });
    constexpr auto wallSwap = frozen::make_map<int, int>({
        {WallID::Unsafe::marble, WallID::Unsafe::granite},
        {WallID::Unsafe::granite, WallID::Unsafe::marble},
    });
    parallelFor(std::views::iota(0, world.getWidth()), [&](int x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            auto blockItr = blockSwap.find(tile.blockID);
            if (blockItr != blockSwap.end()) {
                tile.blockID = blockItr->second;
            } else if (rnd.getCoarseNoise(x, y) > 0) {
                auto oreItr = oreSwap.find(tile.blockID);
                if (oreItr != oreSwap.end()) {
                    tile.blockID = oreItr->second;
                }
            }
            auto wallItr = wallSwap.find(tile.wallID);
            if (wallItr != wallSwap.end()) {
                tile.wallID = wallItr->second;
            }
        }
    });
}
