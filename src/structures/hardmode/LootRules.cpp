#include "structures/hardmode/LootRules.h"

#include "Config.h"
#include "World.h"
#include "ids/ItemID.h"
#include "structures/StructureUtil.h"
#include <cmath>
#include <iostream>
#include <map>

void swapBottledHoney(int x, int y, Item &item, World &world)
{
    if (item.id == ItemID::bottledHoney) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::chest &&
            (tile.frameX == 360 || tile.frameX == 1044)) {
            item.id = ItemID::honeyfin;
        }
    }
}

void swapEvilArrows(int x, int y, Item &item, World &world)
{
    if (item.id == ItemID::jestersArrow || item.id == ItemID::holyArrow) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::chestGroup2 && tile.frameX == 108) {
            item.id = ItemID::cursedArrow;
        } else if (tile.blockID == TileID::chest && tile.frameX == 1548) {
            item.id = ItemID::ichorArrow;
        }
    }
}

void applyHardmodeLoot(World &world)
{
    std::cout << "Compounding interest\n";
    std::map<int, int> itemUpdates{
        {ItemID::copperBar,
         world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                              : ItemID::leadBar},
        {ItemID::tinBar,
         world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                              : ItemID::leadBar},
        {ItemID::ironBar,
         world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                              : ItemID::platinumBar},
        {ItemID::leadBar,
         world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                              : ItemID::platinumBar},
        {ItemID::silverBar, ItemID::meteoriteBar},
        {ItemID::tungstenBar, ItemID::meteoriteBar},
        {ItemID::goldBar,
         world.cobaltVariant == TileID::palladiumOre ? ItemID::palladiumBar
                                                     : ItemID::cobaltBar},
        {ItemID::platinumBar,
         world.cobaltVariant == TileID::cobaltOre ? ItemID::cobaltBar
                                                  : ItemID::palladiumBar},
        {ItemID::meteoriteBar,
         world.mythrilVariant == TileID::orichalcumOre ? ItemID::orichalcumBar
                                                       : ItemID::mythrilBar},
        {ItemID::demoniteBar,
         world.adamantiteVariant == TileID::titaniumOre
             ? ItemID::titaniumBar
             : ItemID::adamantiteBar},
        {ItemID::crimtaneBar,
         world.adamantiteVariant == TileID::adamantiteOre
             ? ItemID::adamantiteBar
             : ItemID::titaniumBar},
        {ItemID::woodenArrow, ItemID::unholyArrow},
        {ItemID::flamingArrow, ItemID::jestersArrow},
        {ItemID::jestersArrow, ItemID::holyArrow},
        {ItemID::silverBullet, ItemID::explodingBullet},
        {ItemID::tungstenBullet, ItemID::explodingBullet},
        {ItemID::lesserHealingPotion, ItemID::healingPotion},
        {ItemID::healingPotion, ItemID::greaterHealingPotion},
        {ItemID::spear, ItemID::swordfish},
        {ItemID::blowpipe, ItemID::blowgun},
        {ItemID::woodenBoomerang, ItemID::enchantedBoomerang},
        {ItemID::mace,
         world.isCrimson ? ItemID::theMeatball : ItemID::ballOHurt},
        {ItemID::boomstick, ItemID::quadBarrelShotgun},
    };
    int skipX = -1;
    int skipY = -1;
    if (world.conf.equipment != 0) {
        double minDist = 9999;
        for (int x = world.spawn.x - 80; x < world.spawn.x + 80; ++x) {
            for (int y = world.spawn.y - 40; y < world.spawn.y + 40; ++y) {
                Tile &tile = world.getTile(x, y);
                if ((tile.blockID == TileID::chest ||
                     tile.blockID == TileID::chestGroup2) &&
                    tile.frameX % 36 == 0 && tile.frameY == 0) {
                    double dist = hypot(world.spawn, {x, y});
                    if (dist < minDist) {
                        minDist = dist;
                        skipX = x;
                        skipY = y;
                    }
                }
            }
        }
    }
    for (auto &chest : world.getChests()) {
        if (chest.x == skipX && chest.y == skipY) {
            continue;
        }
        for (auto &item : chest.items) {
            auto itr = itemUpdates.find(item.id);
            if (itr != itemUpdates.end()) {
                item.id = itr->second;
            }
            swapBottledHoney(chest.x, chest.y, item, world);
            swapEvilArrows(chest.x, chest.y, item, world);
        }
    }
}
