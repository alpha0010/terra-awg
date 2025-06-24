#ifndef DUNGEON_H
#define DUNGEON_H

class World;
class Random;

int computeDungeonCenter(World &world);
void genDungeon(Random &rnd, World &world);

#endif // DUNGEON_H
