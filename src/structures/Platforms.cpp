#include "structures/Platforms.h"

#include "World.h"

namespace Anchor
{
enum {
    flat = 0,
    endcapRight = 18,
    endcapLeft = 36,
    attachLeft = 54,
    attachRight = 72,
    single = 90,
    attachLeftEndcap = 108,
    attachRightEndcap = 126
};
}

void updatePlatformAnchor(int x, int y, World &world)
{
    Tile &cur = world.getTile(x, y);
    if (cur.blockID != TileID::platform) {
        return;
    }
    Tile &prev = world.getTile(x - 1, y);
    Tile &next = world.getTile(x + 1, y);
    if (prev.blockID == TileID::empty) {
        if (next.blockID == TileID::empty) {
            cur.frameX = Anchor::single;
        } else if (next.blockID == TileID::platform) {
            cur.frameX = Anchor::endcapLeft;
        } else {
            cur.frameX = Anchor::attachRightEndcap;
        }
    } else if (prev.blockID == TileID::platform) {
        if (next.blockID == TileID::empty) {
            cur.frameX = Anchor::endcapRight;
        } else if (next.blockID == TileID::platform) {
            cur.frameX = Anchor::flat;
        } else {
            cur.frameX = Anchor::attachRight;
        }
    } else {
        if (next.blockID == TileID::empty) {
            cur.frameX = Anchor::attachLeftEndcap;
        } else if (next.blockID == TileID::platform) {
            cur.frameX = Anchor::attachLeft;
        } else {
            cur.frameX = Anchor::single;
        }
    }
}

void placePlatform(int x, int y, int style, World &world)
{
    Tile &cur = world.getTile(x, y);
    cur.blockID = TileID::platform;
    cur.frameY = style;
    updatePlatformAnchor(x - 1, y, world);
    updatePlatformAnchor(x, y, world);
    updatePlatformAnchor(x + 1, y, world);
}
