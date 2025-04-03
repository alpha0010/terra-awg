#include "structures/Temple.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>
#include <set>

template <typename Func> void iterateTemple(int x, int y, World &world, Func f)
{
    int scanX = 0.025 * world.getWidth();
    int scanY = 0.45 * scanX;
    int startScanY = scanY;
    int stepCtrl = -1;
    for (int i = -scanX; i < scanX; ++i) {
        if (stepCtrl < 0) {
            if (stepCtrl < -3) {
                stepCtrl = i > -30 && i < -15 ? -i * 2 - 1 : 15;
            }
            if (i < 0) {
                startScanY -= 4;
            } else {
                startScanY += 4;
            }
        }
        --stepCtrl;
        int endScanY = scanY - (((i + scanX) / 3) % 2);
        for (int j = startScanY; j < endScanY; ++j) {
            if (!f(x + i, y + j)) {
                return;
            }
        }
    }
}

bool testTempleSelection(int centerX, int centerY, World &world)
{
    std::set<int> avoidBlocks{
        TileID::aetherium,
        TileID::ash,
        TileID::corruptJungleGrass,
        TileID::crimsonJungleGrass,
        TileID::granite,
        TileID::hive,
        TileID::marble,
        TileID::mushroomGrass,
    };
    bool isValid = true;
    iterateTemple(
        centerX,
        centerY,
        world,
        [&avoidBlocks, &isValid, &world](int x, int y) {
            if (avoidBlocks.contains(world.getTile(x, y).blockID)) {
                isValid = false;
                return false;
            }
            return true;
        });
    return isValid;
}

std::pair<int, int> selectTempleCenter(Random &rnd, World &world)
{
    int xMin = world.jungleCenter - 0.083 * world.getWidth();
    int xMax = world.jungleCenter + 0.083 * world.getWidth();
    int yMin = (world.getUndergroundLevel() + world.getCavernLevel()) / 2;
    while (true) {
        int x = rnd.getInt(xMin, xMax);
        int y = rnd.getInt(yMin, world.getUnderworldLevel());
        if (testTempleSelection(x, y, world)) {
            return {x, y};
        }
    }
}

void genTemple(Random &rnd, World &world)
{
    std::cout << "Training acolytes\n";
    auto center = selectTempleCenter(rnd, world);
    iterateTemple(center.first, center.second, world, [&world](int x, int y) {
        Tile &tile = world.getTile(x, y);
        tile.blockID = TileID::lihzahrdBrick;
        tile.wallID = WallID::Unsafe::lihzahrdBrick;
        return true;
    });
}
