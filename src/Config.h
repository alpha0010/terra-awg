#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Random;

enum class GameMode { journey = 3, classic = 0, expert = 1, master = 2 };

struct Config {
    std::string name;
    std::string seed;
    int width;
    int height;
    GameMode mode;
    bool home;
    int equipment;
    bool doubleTrouble;
    bool sunken;
    bool purity;
    bool hardmode;
    bool hardmodeLoot;
    bool patches;
    double patchesHumidity;
    double patchesTemperature;
    double patchesSize;
    double ore;
    double lifeCrystals;
    double manaCrystals;
    double pots;
    double chests;
    double gems;
    double traps;
    double livingTrees;
    double clouds;
    double asteroids;
    double minecartTracks;
    double minecartLength;
    double aetherSize;
    double templeSize;
    double marbleFreq;
    double marbleSize;
    double graniteFreq;
    double graniteSize;
    double glowingMushroomFreq;
    double glowingMushroomSize;
    double hiveFreq;
    double hiveSize;
    double glowingMossFreq;
    double glowingMossSize;
    double snowSize;
    double desertSize;
    double jungleSize;
    bool map;

    std::string getFilename() const;
};

Config readConfig(Random &rnd);

#endif // CONFIG_H
