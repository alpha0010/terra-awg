#include "Config.h"

#include "vendor/INIReader.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

inline const char *confName = "terra-awg.ini";

inline const char *defaultConfigStr = R"([world]
# Name of the generated world.
name = Terra AWG World

# World size:
# - Small: 4200 x 1200
# - Medium: 6400 x 1800
# - Large: 8400 x 2400
# Other values may produce unexpected results.
width = 6400
height = 1800

# Difficulty: journey/classic/expert/master
mode = classic

[extra]
# Output a map preview image.
map = true
)";

GameMode parseGameMode(const std::string &mode)
{
    if (mode == "journey") {
        return GameMode::journey;
    } else if (mode == "expert") {
        return GameMode::expert;
    } else if (mode == "master") {
        return GameMode::master;
    } else if (mode != "classic") {
        std::cout << "Unknown mode '" << mode << "'\n";
    }
    return GameMode::classic;
}

Config readConfig()
{
    Config conf{"Terra AWG World", 6400, 1800, GameMode::classic, true};
    if (!std::filesystem::exists(confName)) {
        std::ofstream out(confName, std::ios::out);
        out.write(defaultConfigStr, std::strlen(defaultConfigStr));
    }
    INIReader reader(confName);
    if (reader.ParseError() < 0) {
        std::cout << "Unable to load config from'" << confName << "'\n";
        return conf;
    }
    conf.name = reader.Get("world", "name", conf.name);
    conf.width = reader.GetInteger("world", "width", conf.width);
    conf.height = reader.GetInteger("world", "height", conf.height);
    conf.mode = parseGameMode(reader.Get("world", "mode", "classic"));
    conf.mapPreview = reader.GetBoolean("extra", "map", conf.mapPreview);
    return conf;
}
