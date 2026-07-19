#include "Presets.h"

#include "Config.h"
#include <iostream>

void applyPreset(const std::string &preset, Config &conf)
{
    if (preset == "bridges") {
        conf.shattered = true;
        conf.minecartTracks = 8.5;
        conf.minecartLength = 0.3;
    } else if (preset == "buried") {
        conf.spawn = SpawnPoint::cavern;
        conf.sunken = true;
        conf.manaCrystals = 2.1;
        conf.pots = 2.5;
        conf.chests = 1.1;
        conf.minecartTracks = 1.8;
        conf.glowingMushroomFreq = 3;
        conf.glowingMushroomSize = 0.775;
        conf.glowingMossFreq = 9;
        conf.glowingMossSize = 0.894;
        conf.endlessRain = true;
    } else if (preset == "downfall") {
        conf.spawn = SpawnPoint::cloud;
        conf.fadedMemories = 0.05;
        conf.bothEvils = true;
        conf.shattered = true;
        conf.meteorites = 5;
        conf.clouds = 1.9;
        conf.evilSize = 1.94;
        conf.graniteFreq = 2.5;
        conf.glowingMushroomFreq = 2.5;
    } else if (preset == "microcosm") {
        conf.width = 1270;
        conf.height = 580;
        conf.aetherSize = 1.1;
        conf.ore = 2;
        conf.lifeCrystals = 1.8;
        conf.manaCrystals = 0.7;
        conf.chests = 1.8;
        conf.trees = 2;
        conf.clouds = 1.7;
        conf.glowingMushroomSize = 1.1;
    } else if (preset == "mountaineer") {
        conf.spawn = SpawnPoint::ocean;
        conf.jaggedRocks = true;
        conf.biomes = BiomeLayout::patches;
        conf.patchesSize = 1.72;
        conf.trees = 1.85;
        conf.lakeSize = 1.3;
        conf.surfaceAmplitude = 2.9;
    } else if (preset == "webbed") {
        conf.cobwebs = 130;
        conf.spiderNestFreq = 3;
        conf.spiderNestSize = 5;
        conf.endlessHalloween = true;
    } else if (preset != "none" && !preset.empty()) {
        std::cout << "Unknown preset '" << preset << "'\n";
    }
}
