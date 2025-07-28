#include "ids/Prefix.h"

namespace PrefixSet
{
std::vector<int> accessory{
    Prefix::none,
    Prefix::hard,
    Prefix::guarding,
    Prefix::armored,
    Prefix::warding,
    Prefix::precise,
    Prefix::lucky,
    Prefix::jagged,
    Prefix::spiked,
    Prefix::angry,
    Prefix::menacing,
    Prefix::brisk,
    Prefix::fleeting,
    Prefix::hasty_accessory,
    Prefix::quick_accessory,
    Prefix::wild,
    Prefix::rash,
    Prefix::intrepid,
    Prefix::violent,
    Prefix::arcane,
};

std::vector<int> universal{
    Prefix::none,
    Prefix::keen,
    Prefix::superior,
    Prefix::forceful,
    Prefix::hurtful,
    Prefix::strong,
    Prefix::unpleasant,
    Prefix::ruthless,
    Prefix::godly,
    Prefix::demonic,
    Prefix::zealous,
};

std::vector<int> common{
    Prefix::none,
    Prefix::keen,
    Prefix::superior,
    Prefix::forceful,
    Prefix::hurtful,
    Prefix::strong,
    Prefix::unpleasant,
    Prefix::ruthless,
    Prefix::godly,
    Prefix::demonic,
    Prefix::zealous,
    Prefix::quick_common,
    Prefix::deadly_common,
    Prefix::agile,
    Prefix::nimble,
    Prefix::murderous,
    Prefix::nasty,
};

std::vector<int> melee{
    Prefix::none,          Prefix::keen,      Prefix::superior,
    Prefix::forceful,      Prefix::hurtful,   Prefix::strong,
    Prefix::unpleasant,    Prefix::ruthless,  Prefix::godly,
    Prefix::demonic,       Prefix::zealous,   Prefix::quick_common,
    Prefix::deadly_common, Prefix::agile,     Prefix::nimble,
    Prefix::murderous,     Prefix::nasty,     Prefix::large,
    Prefix::massive,       Prefix::dangerous, Prefix::savage,
    Prefix::sharp,         Prefix::pointy,    Prefix::bulky,
    Prefix::heavy,         Prefix::light,     Prefix::legendary,
};

std::vector<int> ranged{
    Prefix::none,         Prefix::keen,         Prefix::superior,
    Prefix::forceful,     Prefix::hurtful,      Prefix::strong,
    Prefix::unpleasant,   Prefix::ruthless,     Prefix::godly,
    Prefix::demonic,      Prefix::zealous,      Prefix::quick_common,
    Prefix::agile,        Prefix::nimble,       Prefix::murderous,
    Prefix::nasty,        Prefix::sighted,      Prefix::rapid,
    Prefix::hasty_ranged, Prefix::intimidating, Prefix::deadly_ranged,
    Prefix::staunch,      Prefix::powerful,     Prefix::frenzying,
    Prefix::unreal,
};

std::vector<int> magic{
    Prefix::none,          Prefix::keen,      Prefix::superior,
    Prefix::forceful,      Prefix::hurtful,   Prefix::strong,
    Prefix::unpleasant,    Prefix::ruthless,  Prefix::godly,
    Prefix::demonic,       Prefix::zealous,   Prefix::quick_common,
    Prefix::deadly_common, Prefix::agile,     Prefix::nimble,
    Prefix::murderous,     Prefix::nasty,     Prefix::mystic,
    Prefix::adept,         Prefix::masterful, Prefix::intense,
    Prefix::taboo,         Prefix::celestial, Prefix::furious,
    Prefix::manic,         Prefix::mythical,
};

std::vector<int> magiclownokb{
    Prefix::none,
    Prefix::keen,
    Prefix::hurtful,
    Prefix::demonic,
    Prefix::zealous,
    Prefix::quick_common,
    Prefix::deadly_common,
    Prefix::agile,
    Prefix::nimble,
    Prefix::murderous,
};

void initCelebration()
{
    accessory = {
        Prefix::armored,
        Prefix::warding,
        Prefix::lucky,
        Prefix::angry,
        Prefix::menacing,
        Prefix::hasty_accessory,
        Prefix::quick_accessory,
        Prefix::intrepid,
        Prefix::violent,
        Prefix::arcane,
    };
    universal = {Prefix::superior, Prefix::godly, Prefix::demonic};
    common = {Prefix::superior, Prefix::godly, Prefix::demonic};
    melee = {Prefix::godly, Prefix::savage, Prefix::legendary};
    ranged = {Prefix::godly, Prefix::deadly_ranged, Prefix::unreal};
    magic = {Prefix::godly, Prefix::masterful, Prefix::mythical};
    magiclownokb = {Prefix::demonic, Prefix::deadly_common, Prefix::murderous};
}
} // namespace PrefixSet
