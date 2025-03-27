#ifndef LOOTRULES_H
#define LOOTRULES_H

class Chest;
class Item;
class World;
class Random;

void fillSurfaceChest(Chest &chest, Random &rnd, World &world);
void fillSurfaceFrozenChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundChest(Chest &chest, Random &rnd, World &world);
void fillUndergroundFrozenChest(Chest &chest, Random &rnd, World &world);
void fillCavernChest(Chest &chest, Random &rnd, World &world);
void fillCavernFrozenChest(Chest &chest, Random &rnd, World &world);
void fillDungeonChest(Chest &chest, Random &rnd, World &world);
void fillDungeonBiomeChest(Chest &chest, Random &rnd, Item &&primaryItem);
void fillLihzahrdChest(Chest &chest, Random &rnd);

#endif // LOOTRULES_H
