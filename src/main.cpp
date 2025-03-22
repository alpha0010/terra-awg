#include "DataID.h"
#include "Random.h"
#include "World.h"
#include "Writer.h"
#include "biomes/Base.h"
#include "biomes/Corruption.h"
#include "biomes/Crimson.h"
#include "biomes/Desert.h"
#include "biomes/Forest.h"
#include "biomes/GemCave.h"
#include "biomes/Jungle.h"
#include "biomes/Ocean.h"
#include "biomes/Snow.h"
#include "biomes/SpiderNest.h"
#include "structures/Treasure.h"
#include <array>
#include <chrono>
#include <set>

#define FOREST_BACKGROUNDS 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 31, 51, 71, 72, 73

std::vector<bool> genImportantTileLookup()
{
    std::set<int> tileIds{
        3,   4,   5,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
        21,  24,  26,  27,  28,  29,  31,  33,  34,  35,  36,  42,  49,  50,
        55,  61,  71,  72,  73,  74,  77,  78,  79,  81,  82,  83,  84,  85,
        86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
        100, 101, 102, 103, 104, 105, 106, 110, 113, 114, 125, 126, 128, 129,
        132, 133, 134, 135, 136, 137, 138, 139, 141, 142, 143, 144, 149, 165,
        171, 172, 173, 174, 178, 184, 185, 186, 187, 201, 207, 209, 210, 212,
        215, 216, 217, 218, 219, 220, 227, 228, 231, 233, 235, 236, 237, 238,
        239, 240, 241, 242, 243, 244, 245, 246, 247, 254, 269, 270, 271, 275,
        276, 277, 278, 279, 280, 281, 282, 283, 285, 286, 287, 288, 289, 290,
        291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304,
        305, 306, 307, 308, 309, 310, 314, 316, 317, 318, 319, 320, 323, 324,
        334, 335, 337, 338, 339, 349, 354, 355, 356, 358, 359, 360, 361, 362,
        363, 364, 372, 373, 374, 375, 376, 377, 378, 380, 386, 387, 388, 389,
        390, 391, 392, 393, 394, 395, 405, 406, 410, 411, 412, 413, 414, 419,
        420, 423, 424, 425, 427, 428, 429, 435, 436, 437, 438, 439, 440, 441,
        442, 443, 444, 445, 452, 453, 454, 455, 456, 457, 461, 462, 463, 464,
        465, 466, 467, 468, 469, 470, 471, 475, 476, 480, 484, 485, 486, 487,
        488, 489, 490, 491, 493, 494, 497, 499, 505, 506, 509, 510, 511, 518,
        519, 520, 521, 522, 523, 524, 525, 526, 527, 529, 530, 531, 532, 533,
        538, 542, 543, 544, 545, 547, 548, 549, 550, 551, 552, 553, 554, 555,
        556, 558, 559, 560, 564, 565, 567, 568, 569, 570, 571, 572, 573, 579,
        580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593,
        594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607,
        608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 619, 620, 621, 622,
        623, 624, 629, 630, 631, 632, 634, 637, 639, 640, 642, 643, 644, 645,
        646, 647, 648, 649, 650, 651, 652, 653, 654, 656, 657, 658, 660, 663,
        664, 665};
    std::vector<bool> table;
    for (int i = 0; i < 693; ++i) {
        table.push_back(tileIds.contains(i));
    }
    return table;
}

/**
 * Compatible with C# DateTime binary format.
 */
uint64_t getBinaryTime()
{
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    return ms * 10000 + 621355968000000000ull;
}

int main()
{
    Random rnd;
    World world;

    world.isCrimson = rnd.getBool();
    world.copperVariant = rnd.select({TileID::copperOre, TileID::tinOre});
    world.ironVariant = rnd.select({TileID::ironOre, TileID::leadOre});
    world.silverVariant = rnd.select({TileID::silverOre, TileID::tungstenOre});
    world.goldVariant = rnd.select({TileID::goldOre, TileID::platinumOre});

    genWorldBase(rnd, world);
    genOceans(rnd, world);
    world.planBiomes(rnd);
    genSnow(rnd, world);
    genDesert(rnd, world);
    genJungle(rnd, world);
    genForest(rnd, world);
    if (world.isCrimson) {
        genCrimson(rnd, world);
    } else {
        genCorruption(rnd, world);
    }
    genGemCave(rnd, world);
    genSpiderNest(rnd, world);
    genTreasure(rnd, world);

    Writer w;
    w.putUint32(279); // File format version.
    w.write("relogic", 7);
    w.putUint8(2);    // File type "world".
    w.putUint32(1);   // Save revision.
    w.putBool(false); // Is favorite.
    w.skipBytes(7);   // Unused currently.
    w.putUint16(11);  // File section count.
    uint32_t sectionTablePos = w.tellp();
    w.skipBytes(44); // Reserve space for section pointers.
    std::vector<bool> importantTiles = genImportantTileLookup();
    w.putUint16(importantTiles.size());
    w.putBitVec(importantTiles);
    std::vector<uint32_t> sectionPointers{w.tellp()};

    const char *mapName = "test";
    w.putString(mapName); // Map name.
    w.putString("AWG");   // Seed.
    w.skipBytes(8);       // Generator version. (Is this needed?)
    for (int i = 0; i < 16; ++i) {
        w.putUint8(rnd.getByte()); // GUID.
    }
    int worldID = rnd.getInt(0, std::numeric_limits<int32_t>::max());
    w.putUint32(worldID);                // World ID.
    w.putUint32(0);                      // Map left pixel.
    w.putUint32(16 * world.getWidth());  // Map right pixel.
    w.putUint32(0);                      // Map top pixel.
    w.putUint32(16 * world.getHeight()); // Map bottom pixel.
    w.putUint32(world.getHeight());      // Vertical tiles.
    w.putUint32(world.getWidth());       // Horizontal tiles.
    w.putUint32(0);                      // Game mode.
    w.putBool(false);                    // Drunk world.
    w.putBool(false);                    // For the worthy.
    w.putBool(false);                    // Celebrationmk10.
    w.putBool(false);                    // The constant.
    w.putBool(false);                    // Not the bees.
    w.putBool(false);                    // Don't dig up.
    w.putBool(false);                    // No traps.
    w.putBool(false);                    // Get fixed boi.
    w.putUint64(getBinaryTime());        // Creation time.
    w.putUint8(rnd.getInt(0, 8));        // Moon type.
    for (auto part : rnd.partitionRange(4, world.getWidth())) {
        w.putUint32(part); // Tree style change locations.
    }
    for (int i = 0; i < 4; ++i) {
        w.putUint32(rnd.getInt(0, 5)); // Tree style.
    }
    for (auto part : rnd.partitionRange(4, world.getWidth())) {
        w.putUint32(part); // Cave style change locations.
    }
    for (int i = 0; i < 4; ++i) {
        w.putUint32(rnd.getInt(0, 7)); // Cave style.
    }
    std::vector<int> styles{
        rnd.select({FOREST_BACKGROUNDS}),
        rnd.select({FOREST_BACKGROUNDS}),
        rnd.select({FOREST_BACKGROUNDS}),
        rnd.select({FOREST_BACKGROUNDS}),
        rnd.getInt(0, 4), // Corruption.
        rnd.getInt(0, 5), // Jungle.
        rnd.select({0, 1, 2, 3, 4, 5, 6, 7, 21, 22, 31, 32, 41, 42}), // Snow.
        rnd.getInt(0, 4),                                             // Hallow.
        rnd.getInt(0, 5), // Crimson.
        rnd.getInt(0, 4), // Desert.
        rnd.getInt(0, 5), // Ocean.
        rnd.getInt(0, 3), // Mushroom.
        rnd.getInt(0, 2)  // Underworld.
    };
    w.putUint32(rnd.getInt(0, 3));     // Ice style.
    w.putUint32(styles[5]);            // Jungle style.
    w.putUint32(rnd.getInt(0, 2));     // Underworld style.
    w.putUint32(world.getWidth() / 2); // Spawn X.
    w.putUint32(world.spawnY);         // Spawn Y.
    w.putFloat64(world.getUndergroundLevel());
    w.putFloat64(world.getCavernLevel());
    w.putFloat64(13500);                // Time of day.
    w.putBool(true);                    // Is day.
    w.putUint32(0);                     // Moon phase.
    w.putBool(false);                   // Blood moon.
    w.putBool(false);                   // Eclipse.
    w.putUint32(world.getWidth() / 4);  // Dungeon X.
    w.putUint32(world.getHeight() / 2); // Dungeon Y.
    w.putBool(world.isCrimson);         // Is crimson.
    for (int i = 0; i < 20; ++i) {
        w.putBool(false); // Bosses and npc saves.
    }
    w.putUint8(0);                  // Shadow orbs smashed.
    w.putUint32(0);                 // Alters smashed.
    w.putBool(false);               // Hard mode.
    w.putBool(false);               // After party of doom.
    w.putUint32(0);                 // Invasion delay.
    w.putUint32(0);                 // Invasion size.
    w.putUint32(0);                 // Invasion type.
    w.putFloat64(0);                // Invasion X.
    w.putFloat64(0);                // Slime rain time.
    w.putUint8(0);                  // Sundial cooldown.
    w.putBool(false);               // Raining.
    w.putUint32(0);                 // Rain time left.
    w.putFloat32(0);                // Max rain.
    w.putUint32(-1);                // Cobalt ore variant.
    w.putUint32(-1);                // Mythril ore variant.
    w.putUint32(-1);                // Adamantite ore variant.
    w.putUint8(styles[0]);          // Forest style.
    w.putUint8(styles[4]);          // Corruption style.
    w.putUint8(rnd.getInt(0, 1));   // Underground jungle style.
    w.putUint8(styles[6]);          // Snow style.
    w.putUint8(styles[7]);          // Hallow style.
    w.putUint8(styles[8]);          // Crimson style.
    w.putUint8(styles[9]);          // Desert style.
    w.putUint8(styles[10]);         // Ocean style.
    w.putUint32(0);                 // Cloud background.
    w.putUint16(0);                 // Number of clouds.
    w.putFloat32(0);                // Wind speed.
    w.putUint32(0);                 // Players finished angler quest.
    w.putBool(false);               // Saved angler.
    w.putUint32(rnd.getInt(0, 38)); // Angler quest.
    w.putBool(false);               // Saved stylist.
    w.putBool(false);               // Saved tax collector.
    w.putBool(false);               // Saved golfer.
    w.putUint32(0);                 // Invasion start size.
    w.putUint32(0);                 // Cultist delay.
    w.putUint16(688);               // Mob types.
    for (int i = 0; i < 688; ++i) {
        w.putUint32(0); // Mob kill tally.
    }
    for (int i = 0; i < 21; ++i) {
        w.putBool(false); // Bosses.
    }
    w.putUint32(0);         // Party cooldown.
    w.putUint32(0);         // Partying NPCs.
    w.putBool(false);       // Sandstorm active.
    w.putUint32(0);         // Sandstorm remaining time.
    w.putFloat32(0);        // Sandstorm severity.
    w.putFloat32(0);        // Sandstorm intended severity.
    w.putBool(false);       // Saved tavernkeep.
    w.putBool(false);       // Old one's army tier 1 complete.
    w.putBool(false);       // Old one's army tier 2 complete.
    w.putBool(false);       // Old one's army tier 3 complete.
    w.putUint8(styles[11]); // Mushroom style.
    w.putUint8(styles[12]); // Underworld style (again?).
    for (int i = 1; i < 4; ++i) {
        w.putUint8(styles[i]); // Forest style.
    }
    w.putBool(false); // Used combat book.
    w.putUint32(0);   // Lantern night cooldown.
    w.putBool(false); // Genuine lantern night.
    w.putBool(false); // Manual lantern night.
    w.putBool(false); // Lantern night next is genuine.
    w.putUint32(styles.size());
    for (int style : styles) {
        w.putUint32(style); // Tree top variation.
    }
    w.putBool(false); // Force halloween.
    w.putBool(false); // Force christmas.
    w.putUint32(world.copperVariant);
    w.putUint32(world.ironVariant);
    w.putUint32(world.silverVariant);
    w.putUint32(world.goldVariant);
    for (int i = 0; i < 25; ++i) {
        w.putBool(false); // Bosses and npc saves.
    }
    w.putUint8(0); // Moondial cooldown.
    sectionPointers.push_back(w.tellp());

    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);

            int rle = 0;
            while (y + rle + 1 < world.getHeight() &&
                   tile == world.getTile(x, y + rle + 1)) {
                ++rle;
            }
            y += rle;

            std::array<uint8_t, 4> flags{0, 0, 0, 0};
            if (rle > 255) {
                flags[0] |= 128;
            } else if (rle > 0) {
                flags[0] |= 64;
            }
            if (tile.blockID != TileID::empty) {
                flags[0] |= 2;
                if (tile.blockID > 255) {
                    flags[0] |= 32;
                }
            }
            if (tile.wallID > 0) {
                flags[0] |= 4;
                if (tile.wallID > 255) {
                    flags[2] |= 64;
                }
            }
            switch (tile.liquid) {
            case Liquid::none:
                break;
            case Liquid::water:
                flags[0] |= 8;
                break;
            case Liquid::lava:
                flags[0] |= 16;
                break;
            case Liquid::honey:
                flags[0] |= 24;
                break;
            case Liquid::shimmer:
                flags[0] |= 8;
                flags[2] |= 128;
                break;
            }
            flags[1] |= static_cast<int>(tile.slope) << 4;
            if (tile.wireRed) {
                flags[1] |= 2;
            }
            if (tile.wireBlue) {
                flags[1] |= 4;
            }
            if (tile.wireGreen) {
                flags[1] |= 8;
            }
            if (tile.wireYellow) {
                flags[2] |= 32;
            }
            if (tile.actuator) {
                flags[2] |= 2;
            }
            if (tile.actuated) {
                flags[2] |= 4;
            }
            if (tile.blockPaint > 0) {
                flags[2] |= 8;
            }
            if (tile.wallPaint > 0) {
                flags[2] |= 16;
            }
            if (tile.echoCoatBlock) {
                flags[3] |= 2;
            }
            if (tile.echoCoatWall) {
                flags[3] |= 4;
            }
            if (tile.illuminantBlock) {
                flags[3] |= 8;
            }
            if (tile.illuminantWall) {
                flags[3] |= 16;
            }
            for (int i = 2; i >= 0; --i) {
                if (flags[i + 1] > 0) {
                    flags[i] |= 1;
                }
            }
            for (int i = 0; i < 4; ++i) {
                if (flags[i] == 0 && i > 0) {
                    break;
                }
                w.putUint8(flags[i]);
            }
            if (tile.blockID != TileID::empty) {
                if (tile.blockID > 255) {
                    w.putUint16(tile.blockID);
                } else {
                    w.putUint8(tile.blockID);
                }
                if (importantTiles[tile.blockID]) {
                    w.putUint16(tile.frameX);
                    w.putUint16(tile.frameY);
                }
                if (tile.blockPaint > 0) {
                    w.putUint8(tile.blockPaint);
                }
            }
            if (tile.wallID > 0) {
                w.putUint8(0xff & tile.wallID);
                if (tile.wallPaint > 0) {
                    w.putUint8(tile.wallPaint);
                }
            }
            if (tile.liquid != Liquid::none) {
                w.putUint8(0xff); // Liquid amount.
            }
            if (tile.wallID > 255) {
                w.putUint8(tile.wallID >> 8);
            }
            if (rle > 255) {
                w.putUint16(rle);
            } else if (rle > 0) {
                w.putUint8(rle);
            }
        }
    }
    sectionPointers.push_back(w.tellp());

    w.putUint16(0);  // Number of chests.
    w.putUint16(40); // Slots per chest.
    sectionPointers.push_back(w.tellp());

    w.putUint16(0); // Number of signs.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0);  // Number of shimmered NPCs.
    w.putBool(true); // Begin town NPC record.
    w.putUint32(22); // The guide.
    w.putString(rnd.select({"Andrew", "Asher", "Bradley", "Brandon", "Brett",
                            "Brian",  "Cody",  "Cole",    "Colin",   "Connor",
                            "Daniel", "Dylan", "Garrett", "Harley",  "Jack",
                            "Jacob",  "Jake",  "Jan",     "Jeff",    "Jeffrey",
                            "Joe",    "Kevin", "Kyle",    "Levi",    "Logan",
                            "Luke",   "Marty", "Maxwell", "Ryan",    "Scott",
                            "Seth",   "Steve", "Tanner",  "Trent",   "Wyatt",
                            "Zach"}));  // NPC name.
    w.putFloat32(world.getWidth() / 2); // NPC position X.
    w.putFloat32(world.spawnY);         // NPC position Y.
    w.putBool(true);                    // NPC is homeless.
    w.putUint32(0);                     // NPC home X.
    w.putUint32(0);                     // NPC home Y.
    w.putBool(true);                    // Unknown?
    w.putUint32(0);                     // NPC variation.
    w.putBool(false);                   // End town NPC records.
    w.putBool(false);                   // End pillar records.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0); // Number of tile entities.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0); // Number of weighted pressure plates.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0); // Number of houses.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0); // Bestiary kills.
    w.putUint32(0); // Bestiary seen.
    w.putUint32(0); // Bestiary chatted.
    sectionPointers.push_back(w.tellp());

    w.putBool(true); // New creative power record.
    w.putUint16(0);
    w.putBool(false); // Freeze time.
    w.putBool(true);  // New creative power record.
    w.putUint16(8);
    w.putFloat32(0); // Time rate.
    w.putBool(true); // New creative power record.
    w.putUint16(9);
    w.putBool(false); // Freeze rain status.
    w.putBool(true);  // New creative power record.
    w.putUint16(10);
    w.putBool(false); // Freeze wind status.
    w.putBool(true);  // New creative power record.
    w.putUint16(12);
    w.putFloat32(0); // Difficulty.
    w.putBool(true); // New creative power record.
    w.putUint16(13);
    w.putBool(false); // Freeze infection spread.
    w.putBool(false); // End creative powers records.
    sectionPointers.push_back(w.tellp());

    w.putBool(true); // Begin footer.
    w.putString(mapName);
    w.putUint32(worldID);

    // Finalize.
    w.seekp(sectionTablePos);
    for (auto ptr : sectionPointers) {
        w.putUint32(ptr);
    }
    return 0;
}
