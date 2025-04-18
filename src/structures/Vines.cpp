#include "structures/Vines.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include <iostream>
#include <map>

void genVines(Random &rnd, World &world)
{
    std::cout << "Growing vines\n";
    std::map<int, int> vineTypes{
        {TileID::grass, TileID::vines},
        {TileID::leaf, TileID::vines},
        {TileID::jungleGrass, TileID::jungleVines},
        {TileID::mahoganyLeaf, TileID::vineRope},
        {TileID::corruptGrass, TileID::corruptVines},
        {TileID::corruptJungleGrass, TileID::corruptVines},
        {TileID::crimsonGrass, TileID::crimsonVines},
        {TileID::crimsonJungleGrass, TileID::crimsonVines},
        {TileID::mushroomGrass, TileID::mushroomVines},
        {TileID::ashGrass, TileID::ashVines}};
    parallelFor(std::views::iota(0, world.getWidth()), [&](int x) {
        int vine = TileID::empty;
        int vineLen = 0;
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (vineLen > 0) {
                if (tile.blockID == TileID::empty &&
                    tile.liquid == Liquid::none) {
                    tile.blockID = vine;
                    if (vine == TileID::vineRope) {
                        tile.blockPaint = Paint::lime;
                    }
                    --vineLen;
                    continue;
                } else {
                    vineLen = 0;
                }
            }
            auto vineItr = vineTypes.find(tile.blockID);
            if (vineItr == vineTypes.end() || tile.slope != Slope::none ||
                static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 3 ==
                    0) {
                continue;
            }
            vine = vineItr->second;
            if (vine == TileID::vines && rnd.getCoarseNoise(x, y) > 0.12) {
                vine = TileID::flowerVines;
            }
            vineLen =
                4 + static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 7;
        }
    });
}
