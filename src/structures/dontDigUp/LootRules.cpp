#include "structures/dontDigUp/LootRules.h"

#include "MsgStream.h"
#include "World.h"
#include "ids/ItemID.h"
#include "vendor/frozen/map.h"

void applyDontDigUpLoot(World &world)
{
    msgStrm << "Remixing loot" << std::endl;
    constexpr auto itemUpdates = frozen::make_map<int, int>({
        {ItemID::wandOfSparking, ItemID::magicDagger},
        {ItemID::snowballCannon, ItemID::iceBow},
        {ItemID::aquaScepter, ItemID::bubbleGun},
        {ItemID::flowerOfFire, ItemID::unholyTrident},
    });
    for (auto &chest : world.getChests()) {
        for (auto &item : chest.items) {
            auto itr = itemUpdates.find(item.id);
            if (itr != itemUpdates.end()) {
                item.id = itr->second;
            }
        }
    }
}
