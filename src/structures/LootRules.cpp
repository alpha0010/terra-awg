#include "structures/LootRules.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/ItemID.h"
#include "ids/Prefix.h"

int doFillLoot(
    Chest &chest,
    Random &rnd,
    std::initializer_list<std::pair<double, Item>> loot)
{
    int itemIndex = 0;
    for (auto [probability, item] : loot) {
        if (probability > rnd.getDouble(0, 1)) {
            chest.items[itemIndex] = item;
            ++itemIndex;
            if (item.id == ItemID::flareGun) {
                chest.items[itemIndex] = {
                    ItemID::flare,
                    Prefix::none,
                    rnd.getInt(25, 50)};
                ++itemIndex;
            } else if (item.id == ItemID::livingWoodWand) {
                chest.items[itemIndex] = {ItemID::leafWand, Prefix::none, 1};
                ++itemIndex;
            } else if (item.id == ItemID::livingMahoganyWand) {
                chest.items[itemIndex] = {
                    ItemID::richMahoganyLeafWand,
                    Prefix::none,
                    1};
                ++itemIndex;
            } else if (item.id == ItemID::pharaohsMask) {
                chest.items[itemIndex] = {
                    ItemID::pharaohsRobe,
                    Prefix::none,
                    1};
                ++itemIndex;
            } else if (item.id == ItemID::mushroomHat) {
                chest.items[itemIndex] = {
                    ItemID::mushroomVest,
                    Prefix::none,
                    1};
                ++itemIndex;
                chest.items[itemIndex] = {
                    ItemID::mushroomPants,
                    Prefix::none,
                    1};
                ++itemIndex;
            } else if (item.id == ItemID::tuxedoShirt) {
                chest.items[itemIndex] = {ItemID::tuxedoPants, Prefix::none, 1};
                ++itemIndex;
            }
        }
    }
    return itemIndex;
}

void fillLoot(
    Chest &chest,
    Random &rnd,
    std::initializer_list<std::pair<double, Item>> loot)
{
    double expectedValue = 0;
    for (const auto &row : loot) {
        expectedValue += row.first;
    }
    if (expectedValue > 5) {
        expectedValue = 5 + 0.6 * (expectedValue - 5);
    }
    int minLoot = std::floor(0.9 * expectedValue - 0.5);
    while (doFillLoot(chest, rnd, loot) < minLoot) {
    }
}

std::pair<double, Item> getGlobalItemPrimary(Random &rnd, World &world)
{
    if (world.conf.traps > 14) {
        if (world.conf.doubleTrouble) {
            return {
                0.117,
                {rnd.select({ItemID::gasTrap, ItemID::moonLordLegs}),
                 Prefix::none,
                 1}};
        }
        return {0.05, {ItemID::gasTrap, Prefix::none, 1}};
    }
    return {
        world.conf.doubleTrouble ? 1.0 / 15 : 0,
        {ItemID::moonLordLegs, Prefix::none, 1}};
}

std::pair<double, Item> getGlobalItemPotion(World &world)
{
    return {
        world.conf.doubleTrouble ? 1.0 / 30 : 0,
        {ItemID::redPotion, Prefix::none, 1}};
}

Item getSurfacePrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::spear, rnd.select(PrefixSet::universal), 1},
        {ItemID::blowpipe, rnd.select(PrefixSet::ranged), 1},
        {ItemID::woodenBoomerang, rnd.select(PrefixSet::universal), 1},
        {ItemID::aglet, rnd.select(PrefixSet::accessory), 1},
        {ItemID::climbingClaws, rnd.select(PrefixSet::accessory), 1},
        {ItemID::umbrella, rnd.select(PrefixSet::melee), 1},
        {ItemID::guideToPlantFiberCordage, rnd.select(PrefixSet::accessory), 1},
        {ItemID::wandOfSparking, rnd.select(PrefixSet::magic), 1},
        {ItemID::radar, rnd.select(PrefixSet::accessory), 1},
        {ItemID::stepStool, rnd.select(PrefixSet::accessory), 1},
    });
}

Item getUndergroundPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::bandOfRegeneration, rnd.select(PrefixSet::accessory), 1},
        {ItemID::magicMirror, Prefix::none, 1},
        {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1},
        {ItemID::hermesBoots, rnd.select(PrefixSet::accessory), 1},
        {ItemID::mace, rnd.select(PrefixSet::universal), 1},
        {ItemID::shoeSpikes, rnd.select(PrefixSet::accessory), 1},
    });
}

Item getCavernPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::bandOfRegeneration, rnd.select(PrefixSet::accessory), 1},
        {ItemID::magicMirror, Prefix::none, 1},
        {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1},
        {ItemID::hermesBoots, rnd.select(PrefixSet::accessory), 1},
        {ItemID::shoeSpikes, rnd.select(PrefixSet::accessory), 1},
        {ItemID::flareGun, Prefix::none, 1},
    });
}

Item getFrozenPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::iceBoomerang, rnd.select(PrefixSet::universal), 1},
        {ItemID::iceBlade, rnd.select(PrefixSet::melee), 1},
        {ItemID::iceSkates, rnd.select(PrefixSet::accessory), 1},
        {ItemID::snowballCannon, rnd.select(PrefixSet::ranged), 1},
        {ItemID::blizzardInABottle, rnd.select(PrefixSet::accessory), 1},
        {ItemID::flurryBoots, rnd.select(PrefixSet::accessory), 1},
    });
}

Item getHoneyPrimaryLoot(Random &rnd)
{
    return {
        rnd.pool(
            {ItemID::abeemination,
             ItemID::beeMinecart,
             ItemID::hiveWand,
             ItemID::honeyDispenser}),
        Prefix::none,
        1};
}

Item getIvyPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::feralClaws, rnd.select(PrefixSet::accessory), 1},
        {ItemID::ankletOfTheWind, rnd.select(PrefixSet::accessory), 1},
        {ItemID::staffOfRegrowth, rnd.select(PrefixSet::melee), 1},
        {ItemID::boomstick, rnd.select(PrefixSet::ranged), 1},
        rnd.select<Item>(
            {{ItemID::flowerBoots, rnd.select(PrefixSet::accessory), 1},
             {ItemID::fiberglassFishingPole, Prefix::none, 1}}),
    });
}

Item getMushroomPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::bandOfRegeneration, rnd.select(PrefixSet::accessory), 1},
        {ItemID::magicMirror, Prefix::none, 1},
        {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1},
        {ItemID::hermesBoots, rnd.select(PrefixSet::accessory), 1},
        {ItemID::shroomerang, rnd.select(PrefixSet::universal), 1},
    });
}

Item getWaterPrimaryLoot(Random &rnd)
{
    return rnd.pool<Item>({
        {ItemID::breathingReed, rnd.select(PrefixSet::melee), 1},
        {ItemID::flipper, rnd.select(PrefixSet::accessory), 1},
        {ItemID::trident, rnd.select(PrefixSet::universal), 1},
        {ItemID::innerTube, rnd.select(PrefixSet::accessory), 1},
        {ItemID::waterWalkingBoots, rnd.select(PrefixSet::accessory), 1},
        {ItemID::beachBall, Prefix::none, 1},
    });
}

void fillSurfaceChest(Chest &chest, int torchID, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1, getSurfacePrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select<int>({torchID, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::wood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfaceAshWoodChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1, getSurfacePrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select<int>({ItemID::torch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::ashWood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfaceFrozenChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1, getFrozenPrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {0.05, {ItemID::extractinator, Prefix::none, 1}},
         {0.02, {ItemID::fish, Prefix::none, 1}},
         {1.0 / 7, {ItemID::iceMachine, Prefix::none, 1}},
         {0.2, {ItemID::iceMirror, Prefix::none, 1}},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6,
          {ItemID::frostDaggerfish, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion,
                ItemID::warmthPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select({ItemID::iceTorch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::borealWood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfaceLivingWoodChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1,
          rnd.pool<Item>({
              {ItemID::livingWoodWand, Prefix::none, 1},
              rnd.select<Item>({
                  {ItemID::bugNet, Prefix::none, 1},
                  {ItemID::greenString, rnd.select(PrefixSet::accessory), 1},
                  {ItemID::brownAndSilverDye, Prefix::none, rnd.getInt(3, 6)},
                  {ItemID::greenAndBlackDye, Prefix::none, rnd.getInt(3, 6)},
                  {ItemID::wandOfSparking, rnd.select(PrefixSet::magic), 1},
              }),
              {ItemID::livingWoodWand, Prefix::none, 1},
              {ItemID::finchStaff, rnd.select(PrefixSet::magic), 1},
          })},
         getGlobalItemPrimary(rnd, world),
         {0.1,
          {rnd.select({ItemID::sunflowerMinecart, ItemID::ladybugMinecart}),
           Prefix::none,
           1}},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.4,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::vineRope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select({ItemID::torch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::wood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfacePalmWoodChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1,
          rnd.pool<Item>({
              {ItemID::spear, rnd.select(PrefixSet::universal), 1},
              {ItemID::blowpipe, rnd.select(PrefixSet::ranged), 1},
              {ItemID::woodenBoomerang, rnd.select(PrefixSet::universal), 1},
              {ItemID::aglet, rnd.select(PrefixSet::accessory), 1},
              {ItemID::climbingClaws, rnd.select(PrefixSet::accessory), 1},
              {ItemID::umbrella, rnd.select(PrefixSet::melee), 1},
              {ItemID::wandOfSparking, rnd.select(PrefixSet::magic), 1},
              {ItemID::radar, rnd.select(PrefixSet::accessory), 1},
              {ItemID::stepStool, rnd.select(PrefixSet::accessory), 1},
          })},
         getGlobalItemPrimary(rnd, world),
         {0.05, {ItemID::whitePearl, Prefix::none, 1}},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6,
          {ItemID::boneThrowingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select({ItemID::desertTorch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::palmWood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfacePearlwoodChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1, getSurfacePrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {1.0 / 6, {ItemID::bouncyGlowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::happyGrenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::silkRope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::starAnise}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select<int>({ItemID::hallowedTorch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::pearlwood, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfaceRichMahoganyChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1, getSurfacePrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::poisonedKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::vineRope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion,
                ItemID::ammoReservationPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select({ItemID::jungleTorch, ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {0.5, {ItemID::richMahogany, Prefix::none, rnd.getInt(50, 99)}}});
}

void fillSurfaceWaterChest(Chest &chest, Random &rnd, World &world)
{
    bool nearEdge = chest.x < 350 || chest.x > world.getWidth() - 350;
    fillLoot(
        chest,
        rnd,
        {{1, getWaterPrimaryLoot(rnd)},
         getGlobalItemPrimary(rnd, world),
         {nearEdge ? 0.05 : 0, {ItemID::whitePearl, Prefix::none, 1}},
         {0.5, {ItemID::sandcastleBucket, Prefix::none, 1}},
         {0.5, {ItemID::sharkBait, Prefix::none, 1}},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::ironskinPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion,
                ItemID::miningPotion,
                ItemID::builderPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5,
          {rnd.select(
               {nearEdge ? ItemID::coralTorch : ItemID::glowstick,
                ItemID::bottle}),
           Prefix::none,
           rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
         {nearEdge ? 0.5 : 0,
          {ItemID::coral, Prefix::none, rnd.getInt(3, 5)}}});
}

void fillUndergroundChest(
    Chest &chest,
    int torchID,
    bool isTrapped,
    Random &rnd,
    World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getUndergroundPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {isTrapped ? 1.0 / 3 : 0,
             {ItemID::deadMansSweater, Prefix::none, 1}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {torchID, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundFrozenChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getFrozenPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.02, {ItemID::fish, Prefix::none, 1}},
            {1.0 / 7, {ItemID::iceMachine, Prefix::none, 1}},
            {0.2, {ItemID::iceMirror, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion,
                   ItemID::warmthPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::iceTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundHoneyChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getHoneyPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::honeyBomb, Prefix::none, rnd.getInt(3, 5)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::vineRope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::jungleTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundIvyChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getIvyPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {1.0 / 6, {ItemID::livingMahoganyWand, Prefix::none, 1}},
            {0.1, {ItemID::beeMinecart, Prefix::none, 1}},
            {0.2, {ItemID::honeyDispenser, Prefix::none, 1}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::vineRope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::jungleTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundMushroomChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getMushroomPrimaryLoot(rnd)},
            {1,
             {rnd.pool({ItemID::shroomMinecart, ItemID::mushroomHat}),
              Prefix::none,
              1}},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::mushroomTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundPearlwoodChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getUndergroundPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::silkRope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::jestersArrow, ItemID::starAnise}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::hallowedTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundSandstoneChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.pool<Item>({
                 {ItemID::magicConch, Prefix::none, 1},
                 {ItemID::snakeCharmersFlute, Prefix::none, 1},
                 {ItemID::ancientChisel, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::duneriderBoots, rnd.select(PrefixSet::accessory), 1},
             })},
            getGlobalItemPrimary(rnd, world),
            {0.05,
             {rnd.select({ItemID::whitePearl, ItemID::blackPearl}),
              Prefix::none,
              1}},
            {1.0 / 15, {ItemID::desertMinecart, Prefix::none, 1}},
            {1.0 / 7, {ItemID::encumberingStone, Prefix::none, 1}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::scarabBomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::desertTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundRichMahoganyChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getUndergroundPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::vineRope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::jungleTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillUndergroundWaterChest(Chest &chest, Random &rnd, World &world)
{
    bool nearEdge = chest.x < 350 || chest.x > world.getWidth() - 350;
    fillLoot(
        chest,
        rnd,
        {
            {1, getWaterPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {nearEdge ? 0.6 : 0,
             {rnd.select({ItemID::whitePearl, ItemID::blackPearl}),
              Prefix::none,
              1}},
            {0.5, {ItemID::sandcastleBucket, Prefix::none, 1}},
            {0.5, {ItemID::sharkBait, Prefix::none, 1}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::bomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {nearEdge ? 0.5 : 0,
             {ItemID::coralTorch, Prefix::none, rnd.getInt(10, 20)}},
            {nearEdge ? 0.5 : 0,
             {ItemID::coral, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillCavernChest(
    Chest &chest,
    int torchID,
    bool isTrapped,
    Random &rnd,
    World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getCavernPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {isTrapped ? 1.0 / 3 : 0,
             {ItemID::deadMansSweater, Prefix::none, 1}},
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::throwingKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select<int>({torchID, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernFrozenChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, getFrozenPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.02, {ItemID::fish, Prefix::none, 1}},
            {1.0 / 7, {ItemID::iceMachine, Prefix::none, 1}},
            {0.2, {ItemID::iceMirror, Prefix::none, 1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::frostburnArrow, ItemID::frostDaggerfish}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion,
                   ItemID::warmthPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::iceTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernHoneyChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getHoneyPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {1.0 / 3, {ItemID::honeyBomb, Prefix::none, rnd.getInt(3, 5)}},
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::poisonedKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::jungleTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernIvyChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getIvyPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {1.0 / 6, {ItemID::livingMahoganyWand, Prefix::none, 1}},
            {0.1, {ItemID::beeMinecart, Prefix::none, 1}},
            {0.2, {ItemID::honeyDispenser, Prefix::none, 1}},
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::poisonedKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::jungleTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernMushroomChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getMushroomPrimaryLoot(rnd)},
            {1,
             {rnd.pool({ItemID::shroomMinecart, ItemID::mushroomHat}),
              Prefix::none,
              1}},
            getGlobalItemPrimary(rnd, world),
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::throwingKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::mushroomTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernPearlwoodChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getCavernPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::holyArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::crystalBullet, ItemID::crystalDart}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select<int>({ItemID::hallowedTorch, ItemID::bouncyGlowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernSandstoneChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.pool<Item>({
                 {ItemID::stormSpear, rnd.select(PrefixSet::universal), 1},
                 {ItemID::thunderZapper, rnd.select(PrefixSet::magic), 1},
                 {ItemID::bastStatue, Prefix::none, 1},
             })},
            getGlobalItemPrimary(rnd, world),
            {0.05,
             {rnd.select({ItemID::blackPearl, ItemID::pinkPearl}),
              Prefix::none,
              1}},
            {1.0 / 15, {ItemID::desertMinecart, Prefix::none, 1}},
            {1.0 / 7, {ItemID::encumberingStone, Prefix::none, 1}},
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::boneThrowingKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::desertTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernRichMahoganyChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1, getCavernPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {chest.y < lavaLevel ? 0.05 : 0.15,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::poisonedKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::bottledHoney, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion,
                   ItemID::ammoReservationPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::jungleTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCavernWaterChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    bool nearEdge = chest.x < 350 || chest.x > world.getWidth() - 350;
    fillLoot(
        chest,
        rnd,
        {
            {1, getWaterPrimaryLoot(rnd)},
            getGlobalItemPrimary(rnd, world),
            {nearEdge ? 0.7 : 0,
             {rnd.select({ItemID::whitePearl, ItemID::blackPearl}),
              Prefix::none,
              1}},
            {0.5, {ItemID::sandcastleBucket, Prefix::none, 1}},
            {0.5, {ItemID::sharkBait, Prefix::none, 1}},
            {0.05,
             {chest.y < lavaLevel ? ItemID::extractinator : ItemID::lavaCharm,
              Prefix::none,
              1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::throwingKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {nearEdge ? ItemID::coralTorch : ItemID::glowstick,
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillSkywareChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1,
          rnd.pool<Item>({
              {ItemID::shinyRedBalloon, rnd.select(PrefixSet::accessory), 1},
              {ItemID::starfury, rnd.select(PrefixSet::melee), 1},
              {ItemID::luckyHorseshoe, rnd.select(PrefixSet::accessory), 1},
              {ItemID::celestialMagnet, rnd.select(PrefixSet::accessory), 1},
          })},
         getGlobalItemPrimary(rnd, world),
         {1.0 / 3, {ItemID::skyMill, Prefix::none, 1}},
         {0.025, {ItemID::fledglingWings, rnd.select(PrefixSet::accessory), 1}},
         {1,
          {rnd.pool(
               {ItemID::highPitch,
                ItemID::blessingFromTheHeavens,
                ItemID::constellation,
                ItemID::seeTheWorldForWhatItIs,
                ItemID::loveIsInTheTrashSlot,
                ItemID::eyeOfTheSun}),
           Prefix::none,
           1}},
         {1, {ItemID::cloud, Prefix::none, rnd.getInt(50, 100)}},
         {0.5, {ItemID::fallenStar, Prefix::none, rnd.getInt(3, 5)}},
         {0.2,
          {rnd.select({ItemID::herbBag, ItemID::canOfWorms}),
           Prefix::none,
           rnd.getInt(1, 4)}},
         {0.5,
          {rnd.select(
               {world.copperVariant == TileID::copperOre ? ItemID::copperBar
                                                         : ItemID::tinBar,
                world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                     : ItemID::leadBar}),
           Prefix::none,
           rnd.getInt(3, 10)}},
         {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
         {2.0 / 3,
          {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
           Prefix::none,
           rnd.getInt(25, 50)}},
         getGlobalItemPotion(world),
         {0.5, {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
         {2.0 / 3,
          {rnd.select(
               {ItemID::featherfallPotion,
                ItemID::shinePotion,
                ItemID::nightOwlPotion,
                ItemID::swiftnessPotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {0.5, {ItemID::fairyGlowstick, Prefix::none, rnd.getInt(10, 20)}},
         {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}}});
}

void fillShadowChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.pool<Item>({
                 {ItemID::sunfury, rnd.select(PrefixSet::universal), 1},
                 {ItemID::flowerOfFire, rnd.select(PrefixSet::magic), 1},
                 {ItemID::flamelash, rnd.select(PrefixSet::magic), 1},
                 {ItemID::darkLance, rnd.select(PrefixSet::universal), 1},
                 {ItemID::hellwingBow, rnd.select(PrefixSet::ranged), 1},
             })},
            getGlobalItemPrimary(rnd, world),
            {0.1, {ItemID::demonicHellcart, Prefix::none, 1}},
            {0.1, {ItemID::ornateShadowKey, Prefix::none, 1}},
            {0.1, {ItemID::sliceOfHellCake, Prefix::none, 1}},
            {0.2, {ItemID::treasureMagnet, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.5,
             {rnd.select(
                  {ItemID::meteoriteBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5,
             {rnd.select(
                  {ItemID::hellfireArrow,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBullet
                       : ItemID::tungstenBullet}),
              Prefix::none,
              rnd.getInt(50, 74)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::restorationPotion, Prefix::none, rnd.getInt(15, 20)}},
            {0.75,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::manaRegenerationPotion,
                   ItemID::obsidianSkinPotion,
                   ItemID::magicPowerPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::heartreachPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::gravitationPotion,
                   ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::obsidianSkinPotion,
                   ItemID::battlePotion,
                   ItemID::teleportationPotion,
                   ItemID::infernoPotion,
                   ItemID::lifeforcePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select({ItemID::recallPotion, ItemID::potionOfReturn}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select({ItemID::demonTorch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(2, 4)}},
        });
}

void fillPyramidChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {rnd.pool({ItemID::flyingCarpet, ItemID::sandstormInABottle}),
              rnd.select(PrefixSet::accessory),
              1}},
            getGlobalItemPrimary(rnd, world),
            {0.1, {ItemID::pharaohsMask, Prefix::none, 1}},
            {1.0 / 15, {ItemID::desertMinecart, Prefix::none, 1}},
            {1.0 / 7, {ItemID::encumberingStone, Prefix::none, 1}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.05, {ItemID::flareGun, Prefix::none, 1}},
            {1.0 / 3, {ItemID::scarabBomb, Prefix::none, rnd.getInt(10, 19)}},
            {0.2, {ItemID::angelStatue, Prefix::none, 1}},
            {1.0 / 3, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
            {0.5,
             {rnd.select(
                  {world.ironVariant == TileID::ironOre ? ItemID::ironBar
                                                        : ItemID::leadBar,
                   world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar}),
              Prefix::none,
              rnd.getInt(5, 14)}},
            {0.5,
             {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
              Prefix::none,
              rnd.getInt(25, 49)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::lesserHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::regenerationPotion,
                   ItemID::shinePotion,
                   ItemID::nightOwlPotion,
                   ItemID::swiftnessPotion,
                   ItemID::archeryPotion,
                   ItemID::gillsPotion,
                   ItemID::hunterPotion,
                   ItemID::miningPotion,
                   ItemID::dangersensePotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::desertTorch, Prefix::none, rnd.getInt(10, 20)}},
            {2.0 / 3, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(50, 89)}},
        });
}

void fillWebCoveredChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1, {ItemID::webSlinger, Prefix::none, 1}},
            getGlobalItemPrimary(rnd, world),
            {1, {ItemID::cobweb, Prefix::none, rnd.getInt(10, 29)}},
            {0.05, {ItemID::extractinator, Prefix::none, 1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {world.silverVariant == TileID::silverOre
                       ? ItemID::silverBar
                       : ItemID::tungstenBar,
                   world.goldVariant == TileID::goldOre ? ItemID::goldBar
                                                        : ItemID::platinumBar}),
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {rnd.select({ItemID::flamingArrow, ItemID::throwingKnife}),
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             {rnd.select<int>({ItemID::torch, ItemID::glowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillCrystalChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {world.goldVariant == TileID::goldOre ? ItemID::goldCrown
                                                   : ItemID::platinumCrown,
              Prefix::none,
              1}},
            {0.5,
             {rnd.select({ItemID::diamond, ItemID::amber, ItemID::ruby}),
              Prefix::none,
              rnd.getInt(3, 5)}},
            {0.5,
             {rnd.select({ItemID::emerald, ItemID::sapphire}),
              Prefix::none,
              rnd.getInt(3, 5)}},
            {0.5,
             {rnd.select({ItemID::topaz, ItemID::amethyst}),
              Prefix::none,
              rnd.getInt(3, 5)}},
            {0.3,
             {rnd.select(
                  {ItemID::diamondGemcorn,
                   ItemID::amberGemcorn,
                   ItemID::rubyGemcorn}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.3,
             {rnd.select({ItemID::emeraldGemcorn, ItemID::sapphireGemcorn}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.3,
             {rnd.select({ItemID::topazGemcorn, ItemID::amethystGemcorn}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::lesserLuckPotion,
                   ItemID::nightOwlPotion,
                   ItemID::shinePotion,
                   ItemID::spelunkerPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.4,
             {rnd.select(
                  {ItemID::whiteTorch, ItemID::orangeTorch, ItemID::redTorch}),
              Prefix::none,
              rnd.getInt(10, 20)}},
            {0.4,
             {rnd.select({ItemID::greenTorch, ItemID::blueTorch}),
              Prefix::none,
              rnd.getInt(10, 20)}},
            {0.4,
             {rnd.select({ItemID::yellowTorch, ItemID::purpleTorch}),
              Prefix::none,
              rnd.getInt(10, 20)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillDungeonChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.pool<Item>({
                 {ItemID::muramasa, rnd.select(PrefixSet::melee), 1},
                 {ItemID::cobaltShield, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::aquaScepter, rnd.select(PrefixSet::magic), 1},
                 {ItemID::blueMoon, rnd.select(PrefixSet::universal), 1},
                 {ItemID::magicMissile, rnd.select(PrefixSet::magic), 1},
                 {ItemID::valor, rnd.select(PrefixSet::universal), 1},
                 {ItemID::handgun, rnd.select(PrefixSet::ranged), 1},
             })},
            getGlobalItemPrimary(rnd, world),
            {1.0 / 3, {ItemID::shadowKey, Prefix::none, 1}},
            {0.125, {ItemID::boneWelder, Prefix::none, 1}},
            {0.2,
             {rnd.select({ItemID::suspiciousLookingEye, ItemID::slimeCrown}),
              Prefix::none,
              1}},
            {1.0 / 3, {ItemID::dynamite, Prefix::none, 1}},
            {0.25, {ItemID::jestersArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {world.isCrimson ? ItemID::crimtaneBar : ItemID::demoniteBar,
              Prefix::none,
              rnd.getInt(3, 10)}},
            {0.5,
             {world.silverVariant == TileID::silverOre ? ItemID::silverBullet
                                                       : ItemID::tungstenBullet,
              Prefix::none,
              rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5, {ItemID::healingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::spelunkerPotion,
                   ItemID::featherfallPotion,
                   ItemID::nightOwlPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::archeryPotion,
                   ItemID::gravitationPotion,
                   ItemID::titanPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::thornsPotion,
                   ItemID::waterWalkingPotion,
                   ItemID::invisibilityPotion,
                   ItemID::hunterPotion,
                   ItemID::dangersensePotion,
                   ItemID::teleportationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::recallPotion, Prefix::none, rnd.getInt(1, 2)}},
            {0.5,
             rnd.select<Item>(
                 {{ItemID::boneTorch, Prefix::none, rnd.getInt(15, 29)},
                  {ItemID::spelunkerGlowstick,
                   Prefix::none,
                   rnd.getInt(1, 3)}})},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillDungeonBiomeChest(
    Chest &chest,
    Random &rnd,
    World &world,
    Item &&primaryItem)
{
    fillLoot(
        chest,
        rnd,
        {{1, primaryItem},
         getGlobalItemPrimary(rnd, world),
         {0.5, {ItemID::remnantsOfDevotion, Prefix::none, 1}},
         {1,
          {rnd.select(
               {ItemID::blueBrick, ItemID::greenBrick, ItemID::pinkBrick}),
           Prefix::none,
           rnd.getInt(30, 60)}},
         {1, {ItemID::chlorophyteBar, Prefix::none, rnd.getInt(5, 10)}},
         {1, {ItemID::lifeFruit, Prefix::none, rnd.getInt(1, 2)}},
         getGlobalItemPotion(world),
         {1, {ItemID::greaterHealingPotion, Prefix::none, rnd.getInt(5, 10)}},
         {1,
          {rnd.select(
               {ItemID::wrathPotion,
                ItemID::lifeforcePotion,
                ItemID::ragePotion,
                ItemID::endurancePotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {1, {ItemID::goldCoin, Prefix::none, rnd.getInt(20, 30)}}});
}

void fillLihzahrdChest(Chest &chest, Random &rnd, World &world)
{
    bool brokenTablet = rnd.getBool();
    fillLoot(
        chest,
        rnd,
        {
            {1, {ItemID::lihzahrdPowerCell, Prefix::none, 1}},
            {brokenTablet ? 1.0 : 0.0,
             {ItemID::solarTabletFragment, Prefix::none, rnd.getInt(3, 7)}},
            {brokenTablet ? 0.0 : 1.0, {ItemID::solarTablet, Prefix::none, 1}},
            getGlobalItemPrimary(rnd, world),
            {0.2, {ItemID::lihzahrdFurnace, Prefix::none, 1}},
            {0.2, {ItemID::lihzahrdBrick, Prefix::none, rnd.getInt(30, 60)}},
            {0.2,
             {rnd.select(
                  {ItemID::mechanicalEye,
                   ItemID::mechanicalWorm,
                   ItemID::mechanicalSkull}),
              Prefix::none,
              1}},
            {1.0 / 3, {ItemID::miniNukeII, Prefix::none, rnd.getInt(3, 5)}},
            {0.25,
             {ItemID::chlorophyteArrow, Prefix::none, rnd.getInt(25, 50)}},
            {0.5,
             {rnd.select(
                  {ItemID::chlorophyteBar,
                   ItemID::shroomiteBar,
                   ItemID::spectreBar}),
              Prefix::none,
              rnd.getInt(2, 5)}},
            {0.5,
             {ItemID::chlorophyteBullet, Prefix::none, rnd.getInt(25, 50)}},
            getGlobalItemPotion(world),
            {0.5,
             {ItemID::greaterHealingPotion, Prefix::none, rnd.getInt(3, 5)}},
            {2.0 / 3,
             {rnd.select(
                  {ItemID::lesserLuckPotion,
                   ItemID::heartreachPotion,
                   ItemID::archeryPotion,
                   ItemID::titanPotion,
                   ItemID::magicPowerPotion,
                   ItemID::manaRegenerationPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {1.0 / 3,
             {rnd.select(
                  {ItemID::wrathPotion,
                   ItemID::ragePotion,
                   ItemID::lifeforcePotion,
                   ItemID::endurancePotion,
                   ItemID::luckPotion,
                   ItemID::summoningPotion}),
              Prefix::none,
              rnd.getInt(1, 2)}},
            {0.5, {ItemID::potionOfReturn, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::jungleTorch, Prefix::none, rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(5, 10)}},
        });
}

void fillStarterChestIron(Chest &chest, Random &rnd, World &world)
{
    bool isIron = world.ironVariant == TileID::ironOre;
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {isIron ? ItemID::ironHelmet : ItemID::leadHelmet,
              Prefix::none,
              1}},
            {1,
             {isIron ? ItemID::ironChainmail : ItemID::leadChainmail,
              Prefix::none,
              1}},
            {1,
             {isIron ? ItemID::ironGreaves : ItemID::leadGreaves,
              Prefix::none,
              1}},
            {1,
             {isIron ? ItemID::ironBar : ItemID::leadBar,
              Prefix::none,
              rnd.getInt(80, 85)}},
            {1, {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1}},
            {1, {ItemID::silverCoin, Prefix::none, rnd.getInt(12, 24)}},
        });
}

void fillStarterChestPlatinum(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {world.ironVariant == TileID::ironOre ? ItemID::ironAnvil
                                                   : ItemID::leadAnvil,
              Prefix::none,
              1}},
            {1, {ItemID::platinumHelmet, Prefix::none, 1}},
            {1, {ItemID::platinumChainmail, Prefix::none, 1}},
            {1, {ItemID::platinumGreaves, Prefix::none, 1}},
            {1, {ItemID::platinumBar, Prefix::none, rnd.getInt(95, 100)}},
            {1, {ItemID::grapplingHook, Prefix::none, 1}},
            {1, {ItemID::silverCoin, Prefix::none, rnd.getInt(70, 99)}},
        });
}

void fillStarterChestHellstone(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {world.ironVariant == TileID::ironOre ? ItemID::ironAnvil
                                                   : ItemID::leadAnvil,
              Prefix::none,
              1}},
            {1, {ItemID::moltenHelmet, Prefix::none, 1}},
            {1, {ItemID::moltenBreastplate, Prefix::none, 1}},
            {1, {ItemID::moltenGreaves, Prefix::none, 1}},
            {1, {ItemID::hellstoneBar, Prefix::none, rnd.getInt(85, 90)}},
            {1, {ItemID::hellfireTreads, rnd.select(PrefixSet::accessory), 1}},
            {1, {ItemID::lifeCrystal, Prefix::none, rnd.getInt(2, 4)}},
            {1, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillStarterChestMythril(Chest &chest, Random &rnd, World &world)
{
    int anvilVariant = world.ironVariant == TileID::ironOre ? ItemID::ironAnvil
                                                            : ItemID::leadAnvil;
    if (world.mythrilVariant == TileID::orichalcumOre) {
        fillLoot(
            chest,
            rnd,
            {
                {1, {anvilVariant, Prefix::none, 1}},
                {1, {ItemID::orichalcumHeadgear, Prefix::none, 1}},
                {1, {ItemID::orichalcumMask, Prefix::none, 1}},
                {1, {ItemID::orichalcumHelmet, Prefix::none, 1}},
                {1, {ItemID::orichalcumBreastplate, Prefix::none, 1}},
                {1, {ItemID::orichalcumLeggings, Prefix::none, 1}},
                {1, {ItemID::orichalcumBar, Prefix::none, rnd.getInt(90, 95)}},
                {1,
                 {ItemID::fledglingWings, rnd.select(PrefixSet::accessory), 1}},
                {1, {ItemID::lifeCrystal, Prefix::none, rnd.getInt(4, 6)}},
                {1, {ItemID::goldCoin, Prefix::none, rnd.getInt(3, 6)}},
            });
    } else {
        fillLoot(
            chest,
            rnd,
            {
                {1, {anvilVariant, Prefix::none, 1}},
                {1, {ItemID::mythrilHood, Prefix::none, 1}},
                {1, {ItemID::mythrilHelmet, Prefix::none, 1}},
                {1, {ItemID::mythrilHat, Prefix::none, 1}},
                {1, {ItemID::mythrilChainmail, Prefix::none, 1}},
                {1, {ItemID::mythrilGreaves, Prefix::none, 1}},
                {1, {ItemID::mythrilBar, Prefix::none, rnd.getInt(75, 80)}},
                {1,
                 {ItemID::fledglingWings, rnd.select(PrefixSet::accessory), 1}},
                {1, {ItemID::lifeCrystal, Prefix::none, rnd.getInt(4, 6)}},
                {1, {ItemID::goldCoin, Prefix::none, rnd.getInt(3, 6)}},
            });
    }
}

void fillStarterChest(int level, Chest &chest, Random &rnd, World &world)
{
    switch (level) {
    case ItemID::ironBar:
        fillStarterChestIron(chest, rnd, world);
        break;
    case ItemID::platinumBar:
        fillStarterChestPlatinum(chest, rnd, world);
        break;
    case ItemID::hellstoneBar:
        fillStarterChestHellstone(chest, rnd, world);
        break;
    case ItemID::mythrilBar:
        fillStarterChestMythril(chest, rnd, world);
        break;
    }
}

void fillBarrel(Chest &chest, Random &rnd)
{
    fillLoot(
        chest,
        rnd,
        {
            {0.3, {ItemID::atlanticCod, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::bass, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::flounder, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::neonTetra, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::redSnapper, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::rockLobster, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::salmon, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::shrimp, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::trout, Prefix::none, rnd.getInt(2, 5)}},
            {0.3, {ItemID::tuna, Prefix::none, rnd.getInt(2, 5)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::cobweb, Prefix::none, rnd.getInt(10, 29)}},
        });
}

void fillDresser(Chest &chest, Random &rnd)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             {rnd.pool(
                  {ItemID::tuxedoShirt,
                   ItemID::summerHat,
                   ItemID::robe,
                   ItemID::crimsonCloak,
                   ItemID::redCape,
                   ItemID::winterCape}),
              Prefix::none,
              1}},
            {1, {ItemID::book, Prefix::none, rnd.getInt(3, 5)}},
            {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(1, 2)}},
            {0.5, {ItemID::cobweb, Prefix::none, rnd.getInt(10, 29)}},
        });
}
