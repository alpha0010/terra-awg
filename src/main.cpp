#include "Config.h"
#include "GenRules.h"
#include "Random.h"
#include "World.h"
#include "Writer.h"
#include "map/ImgWriter.h"
#include <array>
#include <chrono>
#include <iostream>

#define FOREST_BACKGROUNDS 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 31, 51, 71, 72, 73
#define SNOW_BACKGROUNDS 0, 1, 2, 3, 4, 5, 6, 7, 21, 22, 31, 32, 41, 42

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

void saveWorldFile(Config &conf, Random &rnd, World &world)
{
    bool isNoTrapsSeed = !conf.doubleTrouble && conf.traps > 14;

    Writer w(conf.getFilename() + ".wld");
    w.putUint32(279); // File format version.
    w.write("relogic", 7);
    w.putUint8(2);    // File type "world".
    w.putUint32(1);   // Save revision.
    w.putBool(false); // Is favorite.
    w.skipBytes(7);   // Unused currently.
    w.putUint16(11);  // File section count.
    uint32_t sectionTablePos = w.tellp();
    w.skipBytes(44); // Reserve space for section pointers.
    w.putUint16(world.getFramedTiles().size());
    w.putBitVec(world.getFramedTiles());
    std::vector<uint32_t> sectionPointers{w.tellp()};

    w.putString(conf.name);     // Map name.
    w.putString(conf.seed);     // Seed.
    w.putUint64(1198295875585); // Generator version.
    for (int i = 0; i < 16; ++i) {
        w.putUint8(rnd.getByte()); // GUID.
    }
    int worldID = rnd.getInt(0, std::numeric_limits<int32_t>::max());
    w.putUint32(worldID);                          // World ID.
    w.putUint32(0);                                // Map left pixel.
    w.putUint32(16 * world.getWidth());            // Map right pixel.
    w.putUint32(0);                                // Map top pixel.
    w.putUint32(16 * world.getHeight());           // Map bottom pixel.
    w.putUint32(world.getHeight());                // Vertical tiles.
    w.putUint32(world.getWidth());                 // Horizontal tiles.
    w.putUint32(static_cast<uint32_t>(conf.mode)); // Game mode.
    w.putBool(conf.doubleTrouble);                 // Drunk world.
    w.putBool(false);                              // For the worthy.
    w.putBool(false);                              // Celebrationmk10.
    w.putBool(false);                              // The constant.
    w.putBool(false);                              // Not the bees.
    w.putBool(false);                              // Don't dig up.
    w.putBool(isNoTrapsSeed);                      // No traps.
    w.putBool(false);                              // Get fixed boi.
    w.putUint64(getBinaryTime());                  // Creation time.
    w.putUint8(rnd.getInt(0, 8));                  // Moon type.
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
    w.putUint32(rnd.getInt(0, 3));     // Ice style.
    w.putUint32(rnd.getInt(0, 5));     // Jungle style.
    w.putUint32(rnd.getInt(0, 2));     // Underworld style.
    w.putUint32(world.getWidth() / 2); // Spawn X.
    w.putUint32(world.spawnY);         // Spawn Y.
    w.putFloat64(world.getUndergroundLevel());
    w.putFloat64(world.getCavernLevel());
    w.putFloat64(13500);         // Time of day.
    w.putBool(true);             // Is day.
    w.putUint32(0);              // Moon phase.
    w.putBool(false);            // Blood moon.
    w.putBool(false);            // Eclipse.
    w.putUint32(world.dungeonX); // Dungeon X.
    w.putUint32(world.dungeonY); // Dungeon Y.
    w.putBool(world.isCrimson);  // Is crimson.
    for (int i = 0; i < 20; ++i) {
        w.putBool(false); // Bosses and npc saves.
    }
    w.putUint8(0); // Shadow orbs smashed.
    w.putUint32(
        conf.hardmode ? conf.doubleTrouble ? 6 : 3 : 0); // Alters smashed.
    w.putBool(conf.hardmode);                            // Hard mode.
    w.putBool(false);                                    // After party of doom.
    w.putUint32(0);                                      // Invasion delay.
    w.putUint32(0);                                      // Invasion size.
    w.putUint32(0);                                      // Invasion type.
    w.putFloat64(0);                                     // Invasion X.
    w.putFloat64(0);                                     // Slime rain time.
    w.putUint8(0);                                       // Sundial cooldown.
    w.putBool(false);                                    // Raining.
    w.putUint32(0);                                      // Rain time left.
    w.putFloat32(0);                                     // Max rain.
    w.putUint32(world.cobaltVariant);                    // Cobalt ore variant.
    w.putUint32(world.mythrilVariant);                   // Mythril ore variant.
    w.putUint32(world.adamantiteVariant);         // Adamantite ore variant.
    w.putUint8(rnd.select({FOREST_BACKGROUNDS})); // Forest style.
    w.putUint8(rnd.getInt(0, 4));                 // Corruption style.
    w.putUint8(rnd.getInt(0, 1));                 // Underground jungle style.
    w.putUint8(rnd.select({SNOW_BACKGROUNDS}));   // Snow style.
    w.putUint8(rnd.getInt(0, 4));                 // Hallow style.
    w.putUint8(rnd.getInt(0, 5));                 // Crimson style.
    w.putUint8(rnd.getInt(0, 4));                 // Desert style.
    w.putUint8(rnd.getInt(0, 5));                 // Ocean style.
    w.putUint32(0);                               // Cloud background.
    w.putUint16(0);                               // Number of clouds.
    w.putFloat32(0);                              // Wind speed.
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
    w.putUint32(0);               // Party cooldown.
    w.putUint32(0);               // Partying NPCs.
    w.putBool(false);             // Sandstorm active.
    w.putUint32(0);               // Sandstorm remaining time.
    w.putFloat32(0);              // Sandstorm severity.
    w.putFloat32(0);              // Sandstorm intended severity.
    w.putBool(false);             // Saved tavernkeep.
    w.putBool(false);             // Old one's army tier 1 complete.
    w.putBool(false);             // Old one's army tier 2 complete.
    w.putBool(false);             // Old one's army tier 3 complete.
    w.putUint8(rnd.getInt(0, 3)); // Mushroom style.
    w.putUint8(rnd.getInt(0, 2)); // Underworld style (again?).
    for (int i = 0; i < 3; ++i) {
        w.putUint8(rnd.select({FOREST_BACKGROUNDS})); // Forest style.
    }
    w.putBool(false); // Used combat book.
    w.putUint32(0);   // Lantern night cooldown.
    w.putBool(false); // Genuine lantern night.
    w.putBool(false); // Manual lantern night.
    w.putBool(false); // Lantern night next is genuine.
    w.putUint32(13);  // Tree top variation count.
    for (int i = 0; i < 4; ++i) {
        w.putUint32(rnd.getInt(0, 5)); // Forest tree tops.
    }
    w.putUint32(rnd.getInt(0, 4));               // Corruption tree tops.
    w.putUint32(rnd.getInt(0, 5));               // Jungle tree tops.
    w.putUint32(rnd.select({SNOW_BACKGROUNDS})); // Snow tree tops.
    w.putUint32(rnd.getInt(0, 4));               // Hallow tree tops.
    w.putUint32(rnd.getInt(0, 5));               // Crimson tree tops.
    w.putUint32(rnd.getInt(0, 4));               // Desert tree tops.
    w.putUint32(rnd.getInt(0, 5));               // Ocean tree tops.
    w.putUint32(rnd.getInt(0, 3));               // Mushroom tree tops.
    w.putUint32(rnd.getInt(0, 5));               // Underworld tree tops.
    w.putBool(false);                            // Force halloween.
    w.putBool(false);                            // Force christmas.
    w.putUint32(world.copperVariant);
    w.putUint32(world.ironVariant);
    w.putUint32(world.silverVariant);
    w.putUint32(world.goldVariant);
    for (int i = 0; i < 25; ++i) {
        w.putBool(false); // Bosses and npc saves.
    }
    w.putUint8(0); // Moondial cooldown.
    sectionPointers.push_back(w.tellp());

    std::vector<std::pair<int, int>> sensors;

    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::sensor) {
                sensors.emplace_back(x, y);
            }

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
                if (world.getFramedTiles()[tile.blockID]) {
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

    w.putUint16(world.getChests().size()); // Number of chests.
    w.putUint16(40);                       // Slots per chest.
    for (auto &chest : world.getChests()) {
        w.putUint32(chest.x); // Chest position X.
        w.putUint32(chest.y); // Chest position Y.
        w.putString("");      // Chest name.
        for (auto &item : chest.items) {
            w.putUint16(item.stack); // Item stack count.
            if (item.stack > 0) {
                w.putUint32(item.id);    // Item ID.
                w.putUint8(item.prefix); // Item prefix.
            }
        }
    }
    sectionPointers.push_back(w.tellp());

    w.putUint16(0); // Number of signs.
    sectionPointers.push_back(w.tellp());

    w.putUint32(0);  // Number of shimmered NPCs.
    w.putBool(true); // Begin town NPC record.
    if (conf.doubleTrouble) {
        w.putUint32(208); // Party Girl.
        w.putString(rnd.select(
            {"Amanda",
             "Bailey",
             "Bambi",
             "Bunny",
             "Candy",
             "Cherry",
             "Dazzle",
             "Destiny",
             "Fantasia",
             "Fantasy",
             "Glitter",
             "Isis",
             "Lexus",
             "Paris",
             "Sparkle",
             "Star",
             "Sugar",
             "Trixy"})); // NPC name.
    } else {
        w.putUint32(22); // The guide.
        w.putString(
            rnd.select({"Andrew", "Asher", "Bradley", "Brandon", "Brett",
                        "Brian",  "Cody",  "Cole",    "Colin",   "Connor",
                        "Daniel", "Dylan", "Garrett", "Harley",  "Jack",
                        "Jacob",  "Jake",  "Jan",     "Jeff",    "Jeffrey",
                        "Joe",    "Kevin", "Kyle",    "Levi",    "Logan",
                        "Luke",   "Marty", "Maxwell", "Ryan",    "Scott",
                        "Seth",   "Steve", "Tanner",  "Trent",   "Wyatt",
                        "Zach"})); // NPC name.
    }
    w.putFloat32(16 * world.getWidth() / 2); // NPC position X.
    w.putFloat32(16 * (world.spawnY - 2));   // NPC position Y.
    w.putBool(true);                         // NPC is homeless.
    w.putUint32(world.getWidth() / 2);       // NPC home X.
    w.putUint32(world.spawnY);               // NPC home Y.
    w.putBool(true);                         // Unknown?
    w.putUint32(0);                          // NPC variation.
    w.putBool(false);                        // End town NPC records.
    w.putBool(false);                        // End pillar records.
    sectionPointers.push_back(w.tellp());

    w.putUint32(sensors.size()); // Number of tile entities.
    for (size_t i = 0; i < sensors.size(); ++i) {
        auto [x, y] = sensors[i];
        Tile &tile = world.getTile(x, y);
        w.putUint8(2);                    // Type: sensor.
        w.putUint32(i);                   // Tile entity ID.
        w.putUint16(x);                   // Tile entity position X.
        w.putUint16(y);                   // Tile entity position Y.
        w.putUint8(1 + tile.frameY / 18); // Sensor type.
        w.putBool(false);                 // Sensor active.
    }
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
    w.putString(conf.name);
    w.putUint32(worldID);

    // Finalize.
    w.seekp(sectionTablePos);
    for (auto ptr : sectionPointers) {
        w.putUint32(ptr);
    }
}

int main()
{
    auto mainStart = std::chrono::high_resolution_clock::now();

    Random rnd;
    Config conf = readConfig(rnd);
    rnd.setSeed(conf.seed);
    World world{conf};

    world.isCrimson = rnd.getBool();
    world.copperVariant = rnd.select({TileID::copperOre, TileID::tinOre});
    world.ironVariant = rnd.select({TileID::ironOre, TileID::leadOre});
    world.silverVariant = rnd.select({TileID::silverOre, TileID::tungstenOre});
    world.goldVariant = rnd.select({TileID::goldOre, TileID::platinumOre});
    if (conf.hardmode) {
        world.cobaltVariant =
            rnd.select({TileID::cobaltOre, TileID::palladiumOre});
        world.mythrilVariant =
            rnd.select({TileID::mythrilOre, TileID::orichalcumOre});
        world.adamantiteVariant =
            rnd.select({TileID::adamantiteOre, TileID::titaniumOre});
    } else {
        world.cobaltVariant = TileID::empty;
        world.mythrilVariant = TileID::empty;
        world.adamantiteVariant = TileID::empty;
    }

    doWorldGen(rnd, world);
    saveWorldFile(conf, rnd, world);

    auto mainEnd = std::chrono::high_resolution_clock::now();
    std::cout << "\nTime: "
              << 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(
                             mainEnd - mainStart)
                             .count()
              << "s\n\n";

    if (conf.map) {
        std::cout << "Rendering map preview\n";
        savePreviewImage(conf.getFilename(), world);
    }
    return 0;
}
