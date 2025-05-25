#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Random;

enum class GameMode { journey = 3, classic = 0, expert = 1, master = 2 };

struct Config {
    std::string name;
    int width;
    int height;
    GameMode mode;
    bool starterHome;
    bool mapPreview;

    std::string getFilename();
};

Config readConfig(Random &rnd);

#endif // CONFIG_H
