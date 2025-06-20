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
    bool patches;
    double patchesHumidity;
    double patchesTemperature;
    double ore;
    double lifeCrystals;
    double manaCrystals;
    double pots;
    double chests;
    double clouds;
    double minecartTracks;
    bool map;

    std::string getFilename() const;
};

Config readConfig(Random &rnd);

#endif // CONFIG_H
