#include "structures/LootRules.h"

#include "Random.h"
#include "World.h"

void fillLoot(
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
            }
        }
    }
}

void fillSurfaceChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1,
          rnd.select<Item>({
              {ItemID::spear, rnd.select(PrefixSet::universal), 1},
              {ItemID::blowpipe, rnd.select(PrefixSet::ranged), 1},
              {ItemID::woodenBoomerang, rnd.select(PrefixSet::universal), 1},
              {ItemID::aglet, rnd.select(PrefixSet::accessory), 1},
              {ItemID::climbingClaws, rnd.select(PrefixSet::accessory), 1},
              {ItemID::umbrella, rnd.select(PrefixSet::melee), 1},
              {ItemID::guideToPlantFiberCordage,
               rnd.select(PrefixSet::accessory),
               1},
              {ItemID::wandOfSparking, rnd.select(PrefixSet::magic), 1},
              {ItemID::radar, rnd.select(PrefixSet::accessory), 1},
              {ItemID::stepStool, rnd.select(PrefixSet::accessory), 1},
          })},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6, {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
         {1.0 / 6, {ItemID::herbBag, Prefix::none, rnd.getInt(1, 4)}},
         {1.0 / 6, {ItemID::canOfWorms, Prefix::none, rnd.getInt(1, 4)}},
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

void fillSurfaceFrozenChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {{1,
          rnd.select<Item>({
              {ItemID::iceBoomerang, rnd.select(PrefixSet::universal), 1},
              {ItemID::iceBlade, rnd.select(PrefixSet::melee), 1},
              {ItemID::iceSkates, rnd.select(PrefixSet::accessory), 1},
              {ItemID::snowballCannon, rnd.select(PrefixSet::ranged), 1},
              {ItemID::blizzardInABottle, rnd.select(PrefixSet::accessory), 1},
              {ItemID::flurryBoots, rnd.select(PrefixSet::accessory), 1},
          })},
         {0.05, {ItemID::extractinator, Prefix::none, 1}},
         {0.02, {ItemID::fish, Prefix::none, 1}},
         {1.0 / 7, {ItemID::iceMachine, Prefix::none, 1}},
         {0.2, {ItemID::iceMirror, Prefix::none, 1}},
         {1.0 / 6, {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
         {1.0 / 6,
          {ItemID::frostDaggerfish, Prefix::none, rnd.getInt(150, 300)}},
         {1.0 / 6, {ItemID::herbBag, Prefix::none, rnd.getInt(1, 4)}},
         {1.0 / 6, {ItemID::canOfWorms, Prefix::none, rnd.getInt(1, 4)}},
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

void fillUndergroundChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.select<Item>({
                 {ItemID::bandOfRegeneration,
                  rnd.select(PrefixSet::accessory),
                  1},
                 {ItemID::magicMirror, Prefix::none, 1},
                 {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::hermesBoots, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::mace, rnd.select(PrefixSet::universal), 1},
                 {ItemID::shoeSpikes, rnd.select(PrefixSet::accessory), 1},
             })},
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
            {0.5, {ItemID::torch, Prefix::none, rnd.getInt(10, 20)}},
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
            {1,
             rnd.select<Item>({
                 {ItemID::iceBoomerang, rnd.select(PrefixSet::universal), 1},
                 {ItemID::iceBlade, rnd.select(PrefixSet::melee), 1},
                 {ItemID::iceSkates, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::snowballCannon, rnd.select(PrefixSet::ranged), 1},
                 {ItemID::blizzardInABottle,
                  rnd.select(PrefixSet::accessory),
                  1},
                 {ItemID::flurryBoots, rnd.select(PrefixSet::accessory), 1},
             })},
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

void fillCavernChest(Chest &chest, Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.select<Item>({
                 {ItemID::bandOfRegeneration,
                  rnd.select(PrefixSet::accessory),
                  1},
                 {ItemID::magicMirror, Prefix::none, 1},
                 {ItemID::cloudInABottle, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::hermesBoots, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::shoeSpikes, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::flareGun, Prefix::none, 1},
             })},
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
             {rnd.select({ItemID::torch, ItemID::glowstick}),
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
            {1,
             rnd.select<Item>({
                 {ItemID::iceBoomerang, rnd.select(PrefixSet::universal), 1},
                 {ItemID::iceBlade, rnd.select(PrefixSet::melee), 1},
                 {ItemID::iceSkates, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::snowballCannon, rnd.select(PrefixSet::ranged), 1},
                 {ItemID::blizzardInABottle,
                  rnd.select(PrefixSet::accessory),
                  1},
                 {ItemID::flurryBoots, rnd.select(PrefixSet::accessory), 1},
             })},
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

void fillDungeonChest(Chest &chest, Random &rnd, World &world)
{
    fillLoot(
        chest,
        rnd,
        {
            {1,
             rnd.select<Item>({
                 {ItemID::muramasa, rnd.select(PrefixSet::melee), 1},
                 {ItemID::cobaltShield, rnd.select(PrefixSet::accessory), 1},
                 {ItemID::aquaScepter, rnd.select(PrefixSet::magic), 1},
                 {ItemID::blueMoon, rnd.select(PrefixSet::universal), 1},
                 {ItemID::magicMissile, rnd.select(PrefixSet::magic), 1},
                 {ItemID::valor, rnd.select(PrefixSet::universal), 1},
                 {ItemID::handgun, rnd.select(PrefixSet::ranged), 1},
             })},
            {1.0 / 3, {ItemID::shadowKey, Prefix::none, 1}},
            {0.125, {ItemID::boneWelder, Prefix::none, 1}},
            {0.2, {ItemID::suspiciousLookingEye, Prefix::none, 1}},
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
             {rnd.select({ItemID::boneTorch, ItemID::spelunkerGlowstick}),
              Prefix::none,
              rnd.getInt(15, 29)}},
            {0.5, {ItemID::goldCoin, Prefix::none, rnd.getInt(1, 2)}},
        });
}

void fillDungeonBiomeChest(Chest &chest, Random &rnd, Item &&primaryItem)
{
    fillLoot(
        chest,
        rnd,
        {{1, primaryItem},
         {1, {ItemID::chlorophyteBar, Prefix::none, rnd.getInt(5, 10)}},
         {1, {ItemID::lifeFruit, Prefix::none, rnd.getInt(1, 2)}},
         {1, {ItemID::greaterHealingPotion, Prefix::none, rnd.getInt(5, 10)}},
         {1, {ItemID::goldCoin, Prefix::none, rnd.getInt(20, 30)}},
         {1,
          {rnd.select(
               {ItemID::wrathPotion,
                ItemID::lifeforcePotion,
                ItemID::ragePotion,
                ItemID::endurancePotion}),
           Prefix::none,
           rnd.getInt(1, 2)}},
         {1,
          {rnd.select(
               {ItemID::blueBrick, ItemID::greenBrick, ItemID::pinkBrick}),
           Prefix::none,
           rnd.getInt(30, 60)}}});
}

void fillLihzahrdChest(Chest &chest, Random &rnd)
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
