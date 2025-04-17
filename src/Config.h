#ifndef CONFIG_H
#define CONFIG_H

#include <string>

enum class GameMode { journey = 3, classic = 0, expert = 1, master = 2 };

struct Config {
    std::string name;
    int width;
    int height;
    GameMode mode;
    bool mapPreview;
};

Config readConfig();

#endif // CONFIG_H
