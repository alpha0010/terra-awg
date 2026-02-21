#include "Config.h"
#include "GenRules.h"
#include "Random.h"
#include "World.h"
#include "Writer.h"
#include "ids/Prefix.h"
#include "map/ImgWriter.h"
#include "structures/StructureUtil.h"
#include <array>
#include <chrono>
#include <iostream>

#define FOREST_BACKGROUNDS 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 31, 51, 71, 72, 73
#define SNOW_BACKGROUNDS 0, 1, 2, 3, 4, 5, 6, 7, 21, 22, 31, 32, 41, 42

namespace NPC
{
enum {
    guide = 22,
    merchant = 17,
    nurse = 18,
    painter = 227,
    dyeTrader = 207,
    zoologist = 633,
    golfer = 588,
    partyGirl = 208,
    angler = 369,
    stylist = 353,
    demolitionist = 38,
    dryad = 20,
    tavernkeep = 550,
    armsDealer = 19,
    goblinTinkerer = 107,
    witchDoctor = 228,
    clothier = 54,
    mechanic = 124,
    taxCollector = 441,
    pirate = 229,
    truffle = 160,
    wizard = 108,
    steampunker = 178,
    cyborg = 209,
    santaClaus = 142,
    princess = 663,
    travelingMerchant = 368,
    oldMan = 37,
    skeletonMerchant = 453,
    townBunny = 656,
    clumsySlime = 680,
    divaSlime = 681,
};
}

std::array townNPCs{
    NPC::guide,        NPC::merchant,        NPC::nurse,
    NPC::painter,      NPC::dyeTrader,       NPC::zoologist,
    NPC::golfer,       NPC::partyGirl,       NPC::angler,
    NPC::stylist,      NPC::demolitionist,   NPC::dryad,
    NPC::tavernkeep,   NPC::armsDealer,      NPC::goblinTinkerer,
    NPC::witchDoctor,  NPC::clothier,        NPC::mechanic,
    NPC::taxCollector, NPC::pirate,          NPC::truffle,
    NPC::wizard,       NPC::steampunker,     NPC::cyborg,
    NPC::santaClaus,   NPC::princess,        NPC::travelingMerchant,
    NPC::oldMan,       NPC::skeletonMerchant};

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

struct NPCDetails {
    std::string name;
    int variation;
};

std::map<int, NPCDetails> determineNPCs(Config &conf, Random &rnd)
{
    bool isWinter =
        conf.tundra ||
        (conf.biomes == BiomeLayout::columns && conf.snowSize > 4.5) ||
        (conf.biomes == BiomeLayout::layers && conf.patchesTemperature < -1) ||
        (conf.biomes == BiomeLayout::patches && conf.patchesTemperature < -0.7);
    std::map<int, NPCDetails> npcs;
    if (conf.celebration) {
        if (conf.evilSize > 3.1) {
            npcs[NPC::dryad] = {"Lilith", 1};
        } else if (conf.dontDigUp) {
            npcs[NPC::taxCollector] = {"Andrew", 1};
        } else if (conf.hiveQueen) {
            npcs[NPC::witchDoctor] = {"Victor", 1};
        } else if (isWinter && conf.endlessChristmas && !conf.skyblock) {
            npcs[NPC::santaClaus] = {"", 1};
        } else {
            npcs[NPC::guide] = {"Andrew", 1};
        }
        if (!conf.skyblock) {
            if (conf.theConstant && !conf.dontDigUp) {
                npcs[NPC::mechanic] = {"Korrie", 1};
            } else {
                npcs[NPC::steampunker] = {"Whitney", 1};
            }
            if (conf.hiveQueen && !conf.dontDigUp) {
                npcs[NPC::merchant] = {"Charles", 1};
            } else {
                npcs[NPC::princess] = {"Yorai", 1};
            }
            if (conf.doubleTrouble) {
                npcs[NPC::painter] = {"Jim", 1};
            } else {
                npcs[NPC::partyGirl] = {"Amanda", 1};
            }
            if (conf.dontDigUp) {
                npcs[NPC::divaSlime] = {"Slimestar", 0};
            } else if (conf.doubleTrouble) {
                npcs[NPC::clumsySlime] = {"Slimefors", 0};
            } else {
                npcs[NPC::townBunny] = {"", 1};
            }
        }
    } else if (conf.evilSize > 3.1) {
        npcs[NPC::dryad] = {"", 1};
    } else if (conf.vampirism) {
        npcs[NPC::zoologist] = {};
    } else if (conf.dontDigUp) {
        npcs[NPC::taxCollector] = {};
    } else if (conf.doubleTrouble && conf.theConstant) {
        npcs[NPC::mechanic] = {};
    } else if (conf.hiveQueen && !conf.theConstant) {
        npcs[NPC::merchant] = {};
    } else if (conf.getFixedBoi) {
        npcs[NPC::demolitionist] = {};
    } else if (conf.doubleTrouble) {
        npcs[NPC::partyGirl] = {};
    } else if (isWinter && conf.endlessChristmas && !conf.skyblock) {
        npcs[NPC::santaClaus] = {};
    } else {
        npcs[NPC::guide] = {};
    }
    for (auto &[npcId, details] : npcs) {
        if (!details.name.empty()) {
            continue;
        }
        switch (npcId) {
        case NPC::guide:
            details.name = rnd.select(
                {"Andrew",  "Asher",   "Bradley", "Brandon", "Brett",  "Brian",
                 "Cody",    "Cole",    "Colin",   "Connor",  "Daniel", "Dylan",
                 "Garrett", "Harley",  "Jack",    "Jacob",   "Jake",   "Jan",
                 "Jeff",    "Jeffrey", "Joe",     "Kevin",   "Kyle",   "Levi",
                 "Logan",   "Luke",    "Marty",   "Maxwell", "Ryan",   "Scott",
                 "Seth",    "Steve",   "Tanner",  "Trent",   "Wyatt",  "Zach"});
            break;
        case NPC::merchant:
            details.name = rnd.select(
                {"Alfred",   "Barney", "Calvin",    "Edmund",   "Edwin",
                 "Eugene",   "Frank",  "Frederick", "Gilbert",  "Gus",
                 "Harold",   "Howard", "Humphrey",  "Isaac",    "Joseph",
                 "Kristian", "Louis",  "Milton",    "Mortimer", "Ralph",
                 "Seymour",  "Walter", "Wilbur"});
            break;
        case NPC::zoologist:
            details.name = rnd.select(
                {"Arien",   "Astra",  "Azaria", "Becca",  "Bindi",
                 "Daphne",  "Dian",   "Ellen",  "Hayley", "Jane",
                 "Juanita", "Lilith", "Lizzy",  "Lori",   "MacKenzie",
                 "Mardy",   "Maria",  "Mollie", "Robyn",  "Tia"});
            break;
        case NPC::partyGirl:
            details.name = rnd.select(
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
                 "Trixy"});
            break;
        case NPC::demolitionist:
            details.name = rnd.select(
                {"Bazdin",  "Beldin", "Boften",  "Darur",    "Dias",
                 "Dolbere", "Dolgen", "Dolgrim", "Duerthen", "Durim",
                 "Fikod",   "Garval", "Gimli",   "Gimut",    "Jarut",
                 "Morthal", "Norkas", "Norsun",  "Oten",     "Ovbere",
                 "Tordak",  "Urist"});
            break;
        case NPC::dryad:
            details.name = rnd.select(
                {"Alalia",   "Alura",     "Ariella", "Caelia",   "Calista",
                 "Celestia", "Chryseis",  "Elysia",  "Emerenta", "Evvie",
                 "Faye",     "Felicitae", "Isis",    "Lunette",  "Nata",
                 "Nissa",    "Rosalva",   "Shea",    "Tania",    "Tatiana",
                 "Xylia"});
            break;
        case NPC::mechanic:
            details.name = rnd.select(
                {"Amy",    "Autumn",    "Brooke",   "Dawn",   "Ella",
                 "Ellen",  "Ginger",    "Jenny",    "Kayla",  "Korrie",
                 "Lauren", "Marshanna", "Meredith", "Nancy",  "Sally",
                 "Selah",  "Selene",    "Shayna",   "Sheena", "Shirlena",
                 "Sophia", "Susana",    "Terra",    "Trisha"});
            break;
        case NPC::taxCollector:
            details.name = rnd.select(
                {"Agnew",
                 "Blanton",
                 "Carroll",
                 "Chester",
                 "Cleveland",
                 "Dwyer",
                 "Fillmore",
                 "Grover",
                 "Harrison",
                 "Herbert",
                 "Lyndon",
                 "McKinly",
                 "Millard",
                 "Ronald",
                 "Rutherford",
                 "Theodore",
                 "Tweed",
                 "Warren"});
            break;
        case NPC::santaClaus:
            details.name = "Santa Claus";
            break;
        case NPC::townBunny:
            details.name = rnd.select(
                {"Babs",
                 "Breadbuns",
                 "Fluffy",
                 "Greg",
                 "Loaf",
                 "Maximus",
                 "Muffin",
                 "Pom"});
            break;
        }
    }
    return npcs;
}

void writeNPC(
    int npcId,
    const std::string &npcName,
    Writer &w,
    Random &rnd,
    World &world,
    int npcVariation = 0)
{
    std::vector<Point> options;
    for (int i = -6; i < 6; ++i) {
        for (int j = -3; j < 3; ++j) {
            if (world.regionPasses(
                    world.spawn.x + i,
                    world.spawn.y - 2 + j,
                    2,
                    3,
                    [](Tile &tile) { return !isSolidBlock(tile.blockID); }) &&
                !world.regionPasses(
                    world.spawn.x + i,
                    world.spawn.y + 1 + j,
                    2,
                    1,
                    [](Tile &tile) { return !isSolidBlock(tile.blockID); })) {
                options.emplace_back(world.spawn.x + i, world.spawn.y + j);
            }
        }
    }
    Point pos = options.empty() ? world.spawn : rnd.select(options);
    w.putBool(true);                // Begin town NPC record.
    w.putUint32(npcId);             // NPC ID.
    w.putString(npcName);           // NPC name.
    w.putFloat32(16 * pos.x - 8);   // NPC position X.
    w.putFloat32(16 * (pos.y - 2)); // NPC position Y.
    w.putBool(true);                // NPC is homeless.
    w.putUint32(pos.x);             // NPC home X.
    w.putUint32(pos.y);             // NPC home Y.
    w.putBool(true);                // Has variation field.
    w.putUint32(npcVariation);      // NPC variation.
    w.putBool(false);               // Despawn if homeless.
}

void saveWorldFile(Config &conf, Random &rnd, World &world)
{
    auto npcs = determineNPCs(conf, rnd);

    Writer w(conf.getFilename() + ".wld");
    w.putUint32(317); // File format version.
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
    w.putUint64(1361504632833); // Generator version.
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
    w.putBool(conf.forTheWorthy);                  // For the worthy.
    w.putBool(conf.celebration);                   // Celebrationmk10.
    w.putBool(conf.theConstant);                   // The constant.
    w.putBool(conf.hiveQueen);                     // Not the bees.
    w.putBool(conf.dontDigUp);                     // Don't dig up.
    w.putBool(conf.traps > 14);                    // No traps.
    w.putBool(conf.getFixedBoi);                   // Get fixed boi.
    w.putBool(conf.skyblock);                      // Skyblock.
    w.putUint64(getBinaryTime());                  // Creation time.
    w.putUint64(getBinaryTime());                  // Last played time.
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
    w.putUint32(rnd.getInt(0, 3)); // Ice style.
    w.putUint32(rnd.getInt(0, 5)); // Jungle style.
    w.putUint32(rnd.getInt(0, 2)); // Underworld style.
    w.putUint32(world.spawn.x);    // Spawn X.
    w.putUint32(world.spawn.y);    // Spawn Y.
    w.putFloat64(world.getUndergroundLevel());
    w.putFloat64(world.getCavernLevel());
    w.putFloat64(13500);          // Time of day.
    w.putBool(true);              // Is day.
    w.putUint32(0);               // Moon phase.
    w.putBool(false);             // Blood moon.
    w.putBool(false);             // Eclipse.
    w.putUint32(world.dungeon.x); // Dungeon X.
    w.putUint32(world.dungeon.y); // Dungeon Y.
    w.putBool(world.isCrimson);   // Is crimson.
    for (int i = 0; i < 13; ++i) {
        w.putBool(false); // Bosses and npc saves.
    }
    w.putBool(npcs.contains(NPC::mechanic)); // Saved mechanic.
    for (int i = 0; i < 6; ++i) {
        w.putBool(false); // Bosses.
    }
    w.putUint8(0); // Shadow orbs smashed.
    w.putUint32(
        conf.hardmode ? conf.doubleTrouble ? 6 : 3 : 0); // Altars smashed.
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
    w.putUint16(rnd.getInt(50, 150));             // Number of clouds.
    w.putFloat32(rnd.getDouble(-0.2, 0.2));       // Wind speed.
    w.putUint32(0);                 // Players finished angler quest.
    w.putBool(false);               // Saved angler.
    w.putUint32(rnd.getInt(0, 38)); // Angler quest.
    w.putBool(false);               // Saved stylist.
    w.putBool(
        npcs.contains(NPC::taxCollector) ||
        (conf.evilSize > 3.1 && conf.dontDigUp)); // Saved tax collector.
    w.putBool(false);                             // Saved golfer.
    w.putUint32(0);                               // Invasion start size.
    w.putUint32(0);                               // Cultist delay.
    w.putUint16(293);                             // Mob types.
    for (int i = 0; i < 293; ++i) {
        w.putUint32(0); // Mob kill tally.
    }
    w.putUint16(293);
    for (int i = 0; i < 293; ++i) {
        w.putUint16(0); // Claimable banners.
    }
    for (int i = 0; i < 19; ++i) {
        w.putBool(false); // Bosses.
    }
    w.putBool(false);             // Manual party.
    w.putBool(conf.celebration);  // Genuine party.
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
    std::vector<bool> unlocks(25);
    unlocks[2] = npcs.contains(NPC::townBunny);
    unlocks[7] = npcs.contains(NPC::merchant);
    unlocks[8] = npcs.contains(NPC::demolitionist);
    unlocks[9] = npcs.contains(NPC::partyGirl);
    unlocks[14] = npcs.contains(NPC::princess);
    unlocks[19] = npcs.contains(NPC::clumsySlime);
    unlocks[20] = npcs.contains(NPC::divaSlime);
    for (bool unlock : unlocks) {
        w.putBool(unlock); // Bosses and npc saves.
    }
    w.putUint8(0);                    // Moondial cooldown.
    w.putBool(conf.endlessHalloween); // Endless halloween.
    w.putBool(conf.endlessChristmas); // Endless christmas.
    w.putBool(conf.vampirism);        // Vampirism.
    w.putBool(conf.evilSize > 3.1);   // Infected world.
    w.putUint32(0);                   // Meteor shower count.
    w.putUint32(0);                   // Coin rain.
    w.putBool(false);                 // Team based spawns.
    w.putUint8(0);                    // Num extra spawn points.
    w.putBool(false);                 // Dual dungeons.
    w.putString("");                  // Worldgen manifest.
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
    for (auto &chest : world.getChests()) {
        w.putUint32(chest.x);            // Chest position X.
        w.putUint32(chest.y);            // Chest position Y.
        w.putString("");                 // Chest name.
        w.putUint32(chest.items.size()); // Chest slots.
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

    std::vector<int> shimmeredNPCs;
    for (auto npcId : townNPCs) {
        if (conf.celebration ||
            (npcs.contains(npcId) && npcs[npcId].variation == 1)) {
            shimmeredNPCs.push_back(npcId);
        }
    }
    std::sort(shimmeredNPCs.begin(), shimmeredNPCs.end());
    w.putUint32(shimmeredNPCs.size()); // Number of shimmered NPCs.
    for (auto npc : shimmeredNPCs) {
        w.putUint32(npc);
    }
    for (const auto &[npcId, details] : npcs) {
        writeNPC(npcId, details.name, w, rnd, world, details.variation);
    }
    w.putBool(false); // End town NPC records.
    w.putBool(false); // End pillar records.
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

    world.isCrimson = conf.evil == EvilBiome::random
                          ? rnd.getBool()
                          : conf.evil == EvilBiome::crimson;
    world.copperVariant = rnd.select({TileID::copperOre, TileID::tinOre});
    world.ironVariant = rnd.select({TileID::ironOre, TileID::leadOre});
    world.silverVariant = rnd.select({TileID::silverOre, TileID::tungstenOre});
    world.goldVariant = rnd.select({TileID::goldOre, TileID::platinumOre});
    if (conf.doubleTrouble) {
        conf.bothEvils = true;
        conf.allOres = true;
    }
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
    if (conf.tundra) {
        conf.patchesTemperature -= 0.5;
        conf.snowSize *= 1.5;
        conf.jungleSize *= 0.8;
        conf.surfaceAmplitude *= 1.25;
    }
    if (conf.celebration) {
        if (conf.spawn == SpawnPoint::normal && !conf.dontDigUp) {
            conf.spawn = SpawnPoint::ocean;
        }
        conf.pots *= 1.6;
        conf.chests *= 1.1;
        conf.gems *= 1.15;
        conf.trees *= 1.1;
        conf.livingTrees *= 1.5;
        conf.minecartTracks *= 1.1;
        conf.minecartLength *= 1.35;
        PrefixSet::initCelebration();
    }
    if (conf.hiveQueen) {
        if (conf.biomes == BiomeLayout::columns) {
            conf.jungleSize *= 2;
        } else if (conf.biomes == BiomeLayout::layers) {
            conf.jungleSize *= 1.35;
        }
    } else if (conf.biomes == BiomeLayout::layers) {
        conf.graniteFreq *= 2;
        conf.graniteSize *= 0.7071;
        conf.marbleFreq *= 2;
        conf.marbleSize *= 0.7071;
    }
    if (conf.forTheWorthy) {
        conf.spiderNestFreq *= 2.5;
        conf.graniteFreq *= 1.7;
        conf.marbleFreq *= 1.85;
        conf.glowingMushroomFreq *= 1.5;
        conf.glowingMushroomSize *= 1.26;
        conf.templeSize *= 1.4;
        conf.glowingMossSize *= 1.225;
        conf.evilSize *= 1.58;
    }
    if (conf.dontDigUp) {
        conf.ascent = true;
        conf.gems *= 0.65;
        conf.evilSize *= 1.5;
    }
    if (conf.ascent) {
        if (conf.spawn == SpawnPoint::normal) {
            conf.spawn = SpawnPoint::underworld;
        }
        conf.minecartTracks *= 1.5;
    }
    if (conf.spawn == SpawnPoint::normal) {
        conf.spawn = SpawnPoint::surface;
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
