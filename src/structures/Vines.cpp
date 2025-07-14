#include "structures/Vines.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <iostream>

enum class ScanState { n, s, se, see, seee, e, ee, eee, eees };

inline constexpr auto attatchTiles = frozen::make_set<int>(
    {TileID::ice,
     TileID::stone,
     TileID::stoneSlab,
     TileID::hive,
     TileID::pearlstone,
     TileID::ebonstone,
     TileID::crimstone,
     TileID::sandstone,
     TileID::granite,
     TileID::marble,
     TileID::hallowedIce,
     TileID::corruptIce,
     TileID::crimsonIce});

ScanState scanTransition(Tile &tile, ScanState state)
{
    if (tile.blockID == TileID::empty) {
        switch (state) {
        case ScanState::n:
            return ScanState::e;
        case ScanState::s:
        case ScanState::eees:
            return ScanState::se;
        case ScanState::se:
            return ScanState::see;
        case ScanState::see:
            return ScanState::seee;
        case ScanState::seee:
        case ScanState::ee:
        case ScanState::eee:
            return ScanState::eee;
        case ScanState::e:
            return ScanState::ee;
        }
    } else if (
        tile.slope == Slope::none && !tile.actuated &&
        attatchTiles.contains(tile.blockID)) {
        switch (state) {
        case ScanState::n:
        case ScanState::s:
        case ScanState::se:
        case ScanState::see:
        case ScanState::e:
        case ScanState::ee:
        case ScanState::eees:
            return ScanState::s;
        case ScanState::seee:
        case ScanState::eee:
            return ScanState::eees;
        }
    }
    return ScanState::n;
}

inline constexpr auto stalactiteTypes = frozen::make_map<int, int>(
    {{TileID::ice, 0},
     {TileID::stone, 54},
     {TileID::hive, 162},
     {TileID::pearlstone, 216},
     {TileID::ebonstone, 270},
     {TileID::crimstone, 324},
     {TileID::sandstone, 378},
     {TileID::granite, 432},
     {TileID::marble, 486},
     {TileID::hallowedIce, 540},
     {TileID::corruptIce, 594},
     {TileID::crimsonIce, 648}});

void placeStalactite(int x, int y, World &world)
{
    int variation = fnv1a32pt(x, y) % 6;
    int frameX = 18 * variation;
    int frameY = 0;
    int height = 2;
    if (variation > 2) {
        frameX -= 54;
        frameY = 72;
        height = 1;
    }
    Tile &probeTile = world.getTile(x, y);
    int paint = probeTile.blockPaint;
    if (probeTile.wallID == WallID::Unsafe::spider &&
        (probeTile.blockID == TileID::stone ||
         probeTile.blockID == TileID::stoneSlab)) {
        frameX += 108;
        frameY = 0;
        height = 2;
        paint = Paint::none;
    } else {
        auto itr = stalactiteTypes.find(probeTile.blockID);
        if (itr == stalactiteTypes.end()) {
            return;
        }
        frameX += itr->second;
        if (itr->first == TileID::hive) {
            if (probeTile.flag == Flag::border) {
                return;
            }
            frameY = 72;
            height = 1;
        }
    }
    for (int j = 0; j < height; ++j) {
        Tile &tile = world.getTile(x, y + j + 1);
        tile.blockID = TileID::stalactite;
        tile.frameX = frameX;
        tile.frameY = 18 * j + frameY;
        tile.blockPaint = paint;
    }
}

inline constexpr auto stalagmiteTypes = frozen::make_map<int, int>(
    {{TileID::stone, 54},
     {TileID::hive, 162},
     {TileID::pearlstone, 216},
     {TileID::ebonstone, 270},
     {TileID::crimstone, 324},
     {TileID::sandstone, 378},
     {TileID::granite, 432},
     {TileID::marble, 486}});

void placeStalagmite(int x, int y, World &world)
{
    Tile &probeTile = world.getTile(x, y);
    auto itr = stalagmiteTypes.find(probeTile.blockID);
    if (itr == stalagmiteTypes.end() ||
        (itr->first == TileID::hive && probeTile.flag == Flag::border)) {
        return;
    }
    int variation = fnv1a32pt(x, y) % 6;
    int frameX = 18 * variation + itr->second;
    int frameY = 36;
    int height = 2;
    if (variation > 2) {
        frameX -= 54;
        frameY = 90;
        height = 1;
    } else if (itr->first == TileID::hive) {
        frameY = 90;
        height = 1;
    }
    for (int j = 0; j < height; ++j) {
        Tile &tile = world.getTile(x, y + j - height);
        tile.blockID = TileID::stalactite;
        tile.frameX = frameX;
        tile.frameY = 18 * j + frameY;
        tile.blockPaint = probeTile.blockPaint;
    }
}

void genVines(Random &rnd, World &world)
{
    std::cout << "Growing vines\n";
    constexpr auto vineTypes = frozen::make_map<int, int>(
        {{TileID::grass, TileID::vines},
         {TileID::leaf, TileID::vines},
         {TileID::jungleGrass, TileID::jungleVines},
         {TileID::mahoganyLeaf, TileID::vineRope},
         {TileID::lihzahrdBrick, TileID::vineRope},
         {TileID::corruptGrass, TileID::corruptVines},
         {TileID::corruptJungleGrass, TileID::corruptVines},
         {TileID::crimsonGrass, TileID::crimsonVines},
         {TileID::crimsonJungleGrass, TileID::crimsonVines},
         {TileID::hallowedGrass, TileID::hallowedVines},
         {TileID::mushroomGrass, TileID::mushroomVines},
         {TileID::ashGrass, TileID::ashVines}});
    constexpr auto dropperTypes = frozen::make_map<int, int>(
        {{TileID::dirt, TileID::waterDrip},
         {TileID::stone, TileID::waterDrip},
         {TileID::jungleGrass, TileID::waterDrip},
         {TileID::cloud, TileID::waterDrip},
         {TileID::rainCloud, TileID::waterDrip},
         {TileID::granite, TileID::waterDrip},
         {TileID::marble, TileID::waterDrip},
         {TileID::livingMahogany, TileID::waterDrip},
         {TileID::mahoganyLeaf, TileID::waterDrip},
         {TileID::lihzahrdBrick, TileID::waterDrip},
         {TileID::ash, TileID::lavaDrip},
         {TileID::ashGrass, TileID::lavaDrip},
         {TileID::obsidianBrick, TileID::lavaDrip},
         {TileID::hellstoneBrick, TileID::lavaDrip},
         {TileID::sand, TileID::sandDrip},
         {TileID::hardenedSand, TileID::sandDrip},
         {TileID::sandstone, TileID::sandDrip},
         {TileID::ebonsand, TileID::sandDrip},
         {TileID::hardenedEbonsand, TileID::sandDrip},
         {TileID::ebonsandstone, TileID::sandDrip},
         {TileID::crimsand, TileID::sandDrip},
         {TileID::hardenedCrimsand, TileID::sandDrip},
         {TileID::crimsandstone, TileID::sandDrip},
         {TileID::pearlsand, TileID::sandDrip},
         {TileID::hardenedPearlsand, TileID::sandDrip},
         {TileID::pearlsandstone, TileID::sandDrip},
         {TileID::hive, TileID::honeyDrip}});
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    parallelFor(std::views::iota(0, world.getWidth()), [&](int x) {
        int vine = TileID::empty;
        int vinePaint = Paint::none;
        int dropper = TileID::empty;
        int vineLen = 0;
        ScanState state = ScanState::n;
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            state = scanTransition(tile, state);
            int randInt = 99999 * (1 + rnd.getFineNoise(x, y));
            if (vineLen > 0) {
                if (tile.blockID == TileID::empty &&
                    (tile.liquid == Liquid::none ||
                     tile.liquid == Liquid::water)) {
                    tile.blockID = vine;
                    tile.blockPaint = vinePaint;
                    state = ScanState::n;
                    --vineLen;
                    continue;
                } else {
                    vineLen = 0;
                }
            } else if (
                dropper != TileID::empty && tile.blockID == TileID::empty &&
                tile.liquid == Liquid::none &&
                randInt % (dropper == TileID::honeyDrip ? 19 : 67) == 0) {
                tile.blockID = dropper == TileID::waterDrip && y > lavaLevel &&
                                       randInt % 5 != 0
                                   ? TileID::lavaDrip
                                   : dropper;
                dropper = TileID::empty;
                state = ScanState::n;
                continue;
            }
            if (state == ScanState::seee && randInt % 7 == 0) {
                placeStalactite(x, y - 3, world);
                state = ScanState::e;
            } else if (state == ScanState::eees && randInt % 11 == 0) {
                placeStalagmite(x, y, world);
            }
            dropper = TileID::empty;
            if (tile.slope != Slope::none || tile.actuated) {
                continue;
            }
            auto vineItr = vineTypes.find(tile.blockID);
            if (vineItr == vineTypes.end() ||
                (tile.blockID == TileID::lihzahrdBrick ? randInt % 29 != 0
                                                       : randInt % 3 == 0)) {
                auto dropperItr = dropperTypes.find(tile.blockID);
                if (dropperItr != dropperTypes.end()) {
                    dropper = dropperItr->second;
                }
                continue;
            }
            vine = vineItr->second;
            if (vine == TileID::vines && rnd.getCoarseNoise(x, y) > 0.12) {
                vine = TileID::flowerVines;
            }
            vinePaint =
                vine == TileID::vineRope ? Paint::lime : tile.blockPaint;
            vineLen = 4 + randInt % 7;
        }
    });
}
