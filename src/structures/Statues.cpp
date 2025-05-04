#include "structures/Statues.h"

#include "World.h"

void placeStatue(int x, int y, int style, World &world)
{
    int offsetX = style / 1000;
    int offsetY = style % 1000 + 162 * (fnv1a32pt(x, y) % 2);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            tile.blockID = TileID::statue;
            tile.frameX = 18 * i + offsetX;
            tile.frameY = 18 * j + offsetY;
        }
    }
}
