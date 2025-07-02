#include "GenRules.h"

#include "Cleanup.h"
#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/Aether.h"
#include "biomes/AshenField.h"
#include "biomes/AsteroidField.h"
#include "biomes/Base.h"
#include "biomes/Cloud.h"
#include "biomes/Corruption.h"
#include "biomes/Crimson.h"
#include "biomes/Desert.h"
#include "biomes/Forest.h"
#include "biomes/GemCave.h"
#include "biomes/GemGrove.h"
#include "biomes/GlowingMoss.h"
#include "biomes/GlowingMushroom.h"
#include "biomes/GraniteCave.h"
#include "biomes/Hive.h"
#include "biomes/Jungle.h"
#include "biomes/MarbleCave.h"
#include "biomes/Ocean.h"
#include "biomes/Snow.h"
#include "biomes/SpiderNest.h"
#include "biomes/Underworld.h"
#include "biomes/doubleTrouble/Corruption.h"
#include "biomes/doubleTrouble/Crimson.h"
#include "biomes/doubleTrouble/ResourceSwap.h"
#include "biomes/hardmode/Hallow.h"
#include "biomes/hardmode/HmOres.h"
#include "biomes/patches/Base.h"
#include "biomes/patches/Cloud.h"
#include "biomes/patches/Hive.h"
#include "biomes/patches/Jungle.h"
#include "structures/BuriedBoat.h"
#include "structures/DesertTomb.h"
#include "structures/Dungeon.h"
#include "structures/Lake.h"
#include "structures/MinecartTracks.h"
#include "structures/MushroomCabin.h"
#include "structures/OceanWreck.h"
#include "structures/Plants.h"
#include "structures/Pyramid.h"
#include "structures/Ruins.h"
#include "structures/SpiderHall.h"
#include "structures/StarterHome.h"
#include "structures/SurfaceIgloo.h"
#include "structures/Temple.h"
#include "structures/TorchArena.h"
#include "structures/Traps.h"
#include "structures/Treasure.h"
#include "structures/Vines.h"
#include "structures/hardmode/LootRules.h"
#include "structures/sunken/Flood.h"
#include <ranges>
#include <set>

enum class Step {
    planBiomes,
    initNoise,
    genWorldBase,
    genOceans,
    genCloud,
    genMarbleCave,
    genSnow,
    genDesert,
    genJungle,
    genForest,
    genAshenField,
    genUnderworld,
    genGlowingMushroom,
    genGraniteCave,
    genHive,
    genAether,
    genCrimson,
    genCorruption,
    applyQueuedEvil,
    genAsteroidField,
    genGemCave,
    genSpiderNest,
    genGlowingMoss,
    genGemGrove,
    genDungeon,
    genTemple,
    genPyramid,
    genDesertTomb,
    genBuriedBoat,
    genSpiderHall,
    genRuins,
    genTorchArena,
    genLake,
    genStarterHome,
    genIgloo,
    genMushroomCabin,
    genOceanWreck,
    genTreasure,
    genPlants,
    genTraps,
    genTracks,
    smoothSurfaces,
    finalizeWalls,
    genVines,
    genGrasses,
    // Double Trouble.
    swapResources,
    genSecondaryCrimson,
    genSecondaryCorruption,
    // Sunken.
    genFlood,
    // Hardmode.
    genHardmodeOres,
    genHallow,
    // Hardmode loot.
    applyHardmodeLoot,
    // Patches.
    initBiomeNoise,
    genWorldBasePatches,
    genCloudPatches,
    genJunglePatches,
    genHivePatches,
};

inline std::array baseBiomeRules{
    Step::planBiomes,
    Step::initNoise,
    Step::genWorldBase,
    Step::genOceans,
    Step::genCloud,
    Step::genMarbleCave,
    Step::genSnow,
    Step::genDesert,
    Step::genJungle,
    Step::genForest,
    Step::genAshenField,
    Step::genUnderworld,
    Step::genGlowingMushroom,
    Step::genGraniteCave,
    Step::genHive,
    Step::genAether,
    Step::genCrimson,
    Step::genCorruption,
    Step::genSecondaryCrimson,
    Step::genSecondaryCorruption,
    Step::applyQueuedEvil,
    Step::genHardmodeOres,
    Step::genHallow,
    Step::swapResources,
    Step::genAsteroidField,
    Step::genGemCave,
    Step::genSpiderNest,
    Step::genGlowingMoss,
    Step::genGemGrove,
};

inline std::array baseStructureRules{
    Step::genDungeon,     Step::genTemple,     Step::genPyramid,
    Step::genDesertTomb,  Step::genBuriedBoat, Step::genSpiderHall,
    Step::genRuins,       Step::genTorchArena, Step::genLake,
    Step::genStarterHome, Step::genIgloo,      Step::genMushroomCabin,
    Step::genOceanWreck,  Step::genTreasure,   Step::applyHardmodeLoot,
    Step::genPlants,      Step::genTraps,      Step::genTracks,
    Step::smoothSurfaces, Step::finalizeWalls, Step::genFlood,
    Step::genVines,       Step::genGrasses,
};

inline std::array patchesBiomeRules{
    Step::initNoise,           Step::initBiomeNoise,
    Step::genWorldBasePatches, Step::genOceans,
    Step::genCloudPatches,     Step::genMarbleCave,
    Step::genJunglePatches,    Step::genForest,
    Step::genAshenField,       Step::genUnderworld,
    Step::genGlowingMushroom,  Step::genGraniteCave,
    Step::genHivePatches,      Step::genAether,
    Step::genCrimson,          Step::genCorruption,
    Step::genSecondaryCrimson, Step::genSecondaryCorruption,
    Step::applyQueuedEvil,     Step::genHardmodeOres,
    Step::genHallow,           Step::swapResources,
    Step::genAsteroidField,    Step::genGemCave,
    Step::genSpiderNest,       Step::genGlowingMoss,
    Step::genGemGrove,
};

#define GEN_STEP(step)                                                         \
    case Step::step:                                                           \
        step(rnd, world);                                                      \
        break;

#define GEN_STEP_WORLD(step)                                                   \
    case Step::step:                                                           \
        step(world);                                                           \
        break;

void doGenStep(Step step, LocationBins &locations, Random &rnd, World &world)
{
    switch (step) {
    case Step::planBiomes:
        world.planBiomes(rnd);
        break;
    case Step::initNoise:
        rnd.initNoise(world.getWidth(), world.getHeight(), 0.07);
        break;
        GEN_STEP(genWorldBase)
        GEN_STEP(genOceans)
        GEN_STEP(genCloud)
        GEN_STEP(genMarbleCave)
        GEN_STEP(genSnow)
        GEN_STEP(genDesert)
        GEN_STEP(genJungle)
        GEN_STEP(genForest)
        GEN_STEP(genAshenField)
        GEN_STEP(genUnderworld)
        GEN_STEP(genGlowingMushroom)
        GEN_STEP(genGraniteCave)
        GEN_STEP(genHive)
        GEN_STEP(genAether)
        GEN_STEP(genCrimson)
        GEN_STEP(genCorruption)
    case Step::applyQueuedEvil:
        for (const auto &applyQueuedEvil : world.queuedEvil) {
            applyQueuedEvil(rnd, world);
        }
        break;
        GEN_STEP(genAsteroidField)
        GEN_STEP(genGemCave)
        GEN_STEP(genSpiderNest)
        GEN_STEP(genGlowingMoss)
        GEN_STEP(genGemGrove)
        GEN_STEP(genDungeon)
        GEN_STEP(genTemple)
        GEN_STEP(genPyramid)
        GEN_STEP(genDesertTomb)
        GEN_STEP(genBuriedBoat)
        GEN_STEP(genSpiderHall)
        GEN_STEP(genRuins)
        GEN_STEP(genTorchArena)
        GEN_STEP(genLake)
        GEN_STEP(genStarterHome)
        GEN_STEP(genIgloo)
        GEN_STEP(genMushroomCabin)
        GEN_STEP(genOceanWreck)
    case Step::genTreasure:
        locations = genTreasure(rnd, world);
        break;
    case Step::genPlants:
        genPlants(locations, rnd, world);
        break;
        GEN_STEP(genTraps)
        GEN_STEP(genTracks)
        GEN_STEP_WORLD(smoothSurfaces)
        GEN_STEP(finalizeWalls)
        GEN_STEP(genVines)
    case Step::genGrasses:
        genGrasses(locations, rnd, world);
        break;
        GEN_STEP(swapResources)
        GEN_STEP(genSecondaryCrimson)
        GEN_STEP(genSecondaryCorruption)
        GEN_STEP_WORLD(genFlood)
        GEN_STEP(genHardmodeOres)
        GEN_STEP(genHallow)
        GEN_STEP_WORLD(applyHardmodeLoot)
    case Step::initBiomeNoise:
        rnd.initBiomeNoise(
            0.00097 / world.conf.patchesSize,
            world.conf.patchesHumidity,
            world.conf.patchesTemperature);
        break;
        GEN_STEP(genWorldBasePatches)
        GEN_STEP(genCloudPatches)
        GEN_STEP(genJunglePatches)
        GEN_STEP(genHivePatches)
    }
}

void doWorldGen(Random &rnd, World &world)
{
    std::set<Step> excludes;
    excludes.insert(world.isCrimson ? Step::genCorruption : Step::genCrimson);
    if (!world.conf.home) {
        excludes.insert(Step::genStarterHome);
    }
    if (world.conf.doubleTrouble) {
        excludes.insert(
            world.isCrimson ? Step::genSecondaryCrimson
                            : Step::genSecondaryCorruption);
    } else {
        excludes.insert(
            {Step::swapResources,
             Step::genSecondaryCrimson,
             Step::genSecondaryCorruption});
    }
    if (!world.conf.sunken) {
        excludes.insert(Step::genFlood);
    }
    if (world.conf.purity) {
        world.surfaceEvilCenter = 0;
        excludes.insert(
            {Step::genCrimson,
             Step::genCorruption,
             Step::genSecondaryCrimson,
             Step::genSecondaryCorruption,
             Step::genHallow});
    }
    if (!world.conf.hardmode) {
        excludes.insert({Step::genHardmodeOres, Step::genHallow});
    }
    if (!world.conf.hardmodeLoot) {
        excludes.insert(Step::applyHardmodeLoot);
    }
    LocationBins locations;
    std::vector<Step> steps;
    if (world.conf.patches) {
        steps.insert(
            steps.end(),
            patchesBiomeRules.begin(),
            patchesBiomeRules.end());
    } else {
        steps.insert(steps.end(), baseBiomeRules.begin(), baseBiomeRules.end());
    }
    steps.insert(
        steps.end(),
        baseStructureRules.begin(),
        baseStructureRules.end());
    for (Step step : steps | std::views::filter([&excludes](Step s) {
                         return !excludes.contains(s);
                     })) {
        doGenStep(step, locations, rnd, world);
    }
}
