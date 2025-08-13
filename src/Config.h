#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Random;

enum class GameMode { journey = 3, classic = 0, expert = 1, master = 2 };

enum class EvilBiome { random, corruption, crimson };

enum class SpawnPoint {
    normal,
    surface,
    ashen,
    cloud,
    ocean,
    cavern,
    underworld,
};

enum class BiomeLayout { columns, layers, patches };

struct Config {
    std::string name;
    std::string seed;
    int width;
    int height;
    GameMode mode;
    EvilBiome evil;
    bool home;
    int equipment;
    SpawnPoint spawn;
    bool unpainted;
    double fadedMemories;
    bool doubleTrouble;
    bool shattered;
    bool sunken;
    bool purity;
    bool hardmode;
    bool hardmodeLoot;
    BiomeLayout biomes;
    double patchesHumidity;
    double patchesTemperature;
    double patchesSize;
    bool celebration;
    bool hiveQueen;
    bool forTheWorthy;
    bool dontDigUp;
    int meteorites;
    double meteoriteSize;
    bool trimEvilTendrils;
    bool avoidantEvil;
    double ore;
    double lifeCrystals;
    double manaCrystals;
    double pots;
    double chests;
    double gems;
    double traps;
    double trees;
    double livingTrees;
    double clouds;
    double asteroids;
    double minecartTracks;
    double minecartLength;
    double aetherSize;
    double dungeonSize;
    double templeSize;
    double evilSize;
    double oceanSize;
    double oceanCaveSize;
    double marbleFreq;
    double marbleSize;
    double graniteFreq;
    double graniteSize;
    double glowingMushroomFreq;
    double glowingMushroomSize;
    double hiveFreq;
    double hiveSize;
    double spiderNestFreq;
    double spiderNestSize;
    double glowingMossFreq;
    double glowingMossSize;
    double snowSize;
    double desertSize;
    double jungleSize;
    double surfaceAmplitude;
    bool map;

    std::string getFilename() const;
};

Config readConfig(Random &rnd);

#endif // CONFIG_H
