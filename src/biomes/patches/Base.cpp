#include "biomes/patches/Base.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "biomes/Base.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>

BiomeData computeBiomeData(int x, int y, Random &rnd)
{
    double snow = std::clamp(
        std::min(
            -10.31 * rnd.getTemperature(x, y) - 4.712,
            6.67 * rnd.getHumidity(x, y) + 4.67),
        0.0,
        1.0);
    double desert = std::clamp(
        std::min(
            -10.31 * rnd.getHumidity(x, y) - 4.712,
            6.67 * rnd.getTemperature(x, y) + 4.67),
        0.0,
        1.0);
    double jungle = std::clamp(
        14.706 * std::min(rnd.getTemperature(x, y), rnd.getHumidity(x, y)) -
            0.206,
        0.0,
        1.0);
    double underworld =
        std::clamp(14.286 * rnd.getTemperature(x, y) - 14.5, 0.0, 1.0);
    if (underworld > 0.5) {
        double mult = 2 - 2 * underworld;
        snow *= mult;
        desert *= mult;
        jungle *= mult;
    }
    double total = snow + desert + jungle + underworld;
    double forest = std::clamp(1 - total, 0.0, 1.0);
    total += forest;
    total = 1 / total;
    snow *= total;
    desert *= total;
    jungle *= total;
    forest *= total;
    underworld *= total;

    std::array biomes = std::to_array({
        std::pair{forest, Biome::forest},
        {desert, Biome::desert},
        {jungle, Biome::jungle},
        {snow, Biome::snow},
        {underworld, Biome::underworld},
    });
    std::vector<std::pair<double, Biome>> activeBiomes;
    if (fnv1a32pt(x, y) % 37 > 4) {
        std::vector<std::pair<double, Biome>> looseActiveBiomes;
        for (auto [prob, biome] : biomes) {
            if (prob > 0.02) {
                activeBiomes.emplace_back(prob, biome);
            }
            if (prob > 0.1) {
                looseActiveBiomes.emplace_back(prob, biome);
            }
        }
        if (looseActiveBiomes.size() == 2) {
            activeBiomes = looseActiveBiomes;
        }
    }
    Biome active = Biome::forest;
    if (activeBiomes.size() == 2) {
        int offset =
            static_cast<int>(99999 * (1 + rnd.getFineNoise(0, 0))) % 997;
        active = activeBiomes[0].first +
                             0.6 * rnd.getFineNoise(x + offset, y + offset) >
                         0.5
                     ? activeBiomes[0].second
                     : activeBiomes[1].second;
    } else {
        int quantFactor = 1400;
        int target = fnv1a32pt(x, y) % (quantFactor - 1);
        int accu = 0;
        for (auto [prob, biome] : biomes) {
            accu += prob * quantFactor;
            if (accu > target) {
                active = biome;
                break;
            }
        }
    }
    return {active, forest, snow, desert, jungle, underworld};
}

void identifySurfaceBiomes(World &world)
{
    double maxDesert = 0;
    double maxJungle = 0;
    double maxSnow = 0;
    double curDesert = 0;
    double curJungle = 0;
    double curSnow = 0;
    int desertCenter = 0;
    int jungleCenter = 0;
    int snowCenter = 0;
    auto scanColumn = [&](int x, bool apply) {
        curDesert *= 0.99;
        curJungle *= 0.99;
        curSnow *= 0.99;
        int surface = world.getSurfaceLevel(x);
        for (int y = surface; y < surface + 30; ++y) {
            const BiomeData &biome = world.getBiome(x, y);
            curDesert += 0.01 * biome.desert;
            curJungle += 0.01 * biome.jungle;
            curSnow += 0.01 * biome.snow;
        }
        if (!apply) {
            return;
        }
        if (curDesert > maxDesert) {
            maxDesert = curDesert;
            desertCenter = x;
        }
        if (curJungle > maxJungle) {
            maxJungle = curJungle;
            jungleCenter = x;
        }
        if (curSnow > maxSnow) {
            maxSnow = curSnow;
            snowCenter = x;
        }
    };
    for (int x = 50; x < world.getWidth() - 200; ++x) {
        scanColumn(x, x > 350);
    }
    world.desertCenter = desertCenter;
    world.jungleCenter = jungleCenter;
    world.snowCenter = snowCenter;

    maxDesert = 0;
    maxJungle = 0;
    maxSnow = 0;
    curDesert = 0;
    curJungle = 0;
    curSnow = 0;
    for (int x = world.getWidth() - 50; x > 200; --x) {
        scanColumn(x, x < world.getWidth() - 350);
    }

    auto mergeResults =
        [threshold =
             std::min<int>(0.22 * world.getWidth(), 1200)](double a, double b) {
            return std::abs(a - b) < threshold ? std::midpoint(a, b)
                                               : std::max(a - 150, 0.0);
        };
    world.desertCenter = mergeResults(world.desertCenter, desertCenter);
    world.jungleCenter = mergeResults(world.jungleCenter, jungleCenter);
    world.snowCenter = mergeResults(world.snowCenter, snowCenter);
}

void genWorldBasePatches(Random &rnd, World &world)
{
    std::cout << "Generating base terrain\n";
    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            world.getBiome(x, y) = computeBiomeData(x, y, rnd);
        }
    });
    applyBaseTerrain(rnd, world);
    identifySurfaceBiomes(world);
}
