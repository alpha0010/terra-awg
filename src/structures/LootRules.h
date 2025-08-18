#ifndef LOOTRULES_H
#define LOOTRULES_H

class Chest;
class Item;
class World;
class Random;

enum class Depth { surface, underground, cavern };

bool fuzzyIsSurfaceChest(int x, int y, World &world);
Depth getChestDepth(int x, int y, World &world);

void fillChest(
    Chest &chest,
    Depth depth,
    int torchID,
    bool isTrapped,
    Random &rnd,
    World &world);
void fillAshWoodChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillFrozenChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillHoneyChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillIvyChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillMushroomChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillDesertChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillPearlwoodChest(Chest &chest, Depth depth, Random &rnd, World &world);
void fillRichMahoganyChest(
    Chest &chest,
    Depth depth,
    Random &rnd,
    World &world);
void fillWaterChest(Chest &chest, Depth depth, Random &rnd, World &world);

void fillLivingWoodChest(Chest &chest, Random &rnd, World &world);
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
    int dye,
    Item &&primaryItem);
void fillLihzahrdChest(Chest &chest, Random &rnd, World &world);
void fillStarterChest(int level, Chest &chest, Random &rnd, World &world);
void fillBarrel(Chest &chest, Random &rnd);
void fillDresser(Chest &chest, Random &rnd);

#endif // LOOTRULES_H
