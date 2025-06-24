#ifndef LOOTRULES_H
#define LOOTRULES_H

class Chest;
class Item;
class World;
class Random;

void fillSurfaceChest(Chest &chest, int torchID, Random &rnd, World &world);
void fillSurfaceAshWoodChest(Chest &chest, Random &rnd, World &world);
void fillSurfaceFrozenChest(Chest &chest, Random &rnd, World &world);
void fillSurfaceLivingWoodChest(Chest &chest, Random &rnd, World &world);
void fillSurfacePalmWoodChest(Chest &chest, Random &rnd, World &world);
void fillSurfacePearlwoodChest(Chest &chest, Random &rnd, World &world);
void fillSurfaceRichMahoganyChest(Chest &chest, Random &rnd, World &world);
void fillSurfaceWaterChest(Chest &chest, Random &rnd, World &world);

void fillUndergroundChest(
    Chest &chest,
    int torchID,
    bool isTrapped,
    Random &rnd,
    World &world);
void fillUndergroundFrozenChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundHoneyChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundIvyChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundMushroomChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundSandstoneChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundPearlwoodChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundRichMahoganyChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundWaterChest(Chest &chest, Random &rnd, World &world);

void fillCavernChest(
    Chest &chest,
    int torchID,
    bool isTrapped,
    Random &rnd,
    World &world);
void fillCavernFrozenChest(Chest &chest, Random &rnd, World &world);
void fillCavernHoneyChest(Chest &chest, Random &rnd, World &world);
void fillCavernIvyChest(Chest &chest, Random &rnd, World &world);
void fillCavernMushroomChest(Chest &chest, Random &rnd, World &world);
void fillCavernSandstoneChest(Chest &chest, Random &rnd, World &world);
void fillCavernPearlwoodChest(Chest &chest, Random &rnd, World &world);
void fillCavernRichMahoganyChest(Chest &chest, Random &rnd, World &world);
void fillCavernWaterChest(Chest &chest, Random &rnd, World &world);

void fillSkywareChest(Chest &chest, Random &rnd, World &world);
void fillShadowChest(Chest &chest, Random &rnd, World &world);
void fillPyramidChest(Chest &chest, Random &rnd, World &world);
void fillWebCoveredChest(Chest &chest, Random &rnd, World &world);
void fillCrystalChest(Chest &chest, Random &rnd, World &world);
void fillDungeonChest(Chest &chest, Random &rnd, World &world);
void fillDungeonBiomeChest(
    Chest &chest,
    Random &rnd,
    World &world,
    Item &&primaryItem);
void fillLihzahrdChest(Chest &chest, Random &rnd, World &world);
void fillStarterChest(int level, Chest &chest, Random &rnd, World &world);
void fillBarrel(Chest &chest, Random &rnd);
void fillDresser(Chest &chest, Random &rnd);

#endif // LOOTRULES_H
