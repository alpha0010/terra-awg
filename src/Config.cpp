#include "Config.h"

#include "Random.h"
#include "ids/ItemID.h"
#include "vendor/INIReader.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

inline const char *confName = "terra-awg.ini";

inline const char *defaultConfigStr = R"([world]
# Name of the generated world.
# Use the name:
#   RANDOM
# for a randomly generated name.
name = Terra AWG World

# World generation seed. (Other settings must match for seed to
# produce an equivalent world.)
# Use the value:
#   RANDOM
# for a randomly generated seed.
#
# Note that that vanilla secret seeds have no special meaning here. To
# integrate with secret seeds, see the related settings below.
seed = RANDOM

# World size:
# - Small: 4200 x 1200
# - Medium: 6400 x 1800
# - Large: 8400 x 2400
# Other values may produce unexpected results.
width = 6400
height = 1800

# Difficulty: journey/classic/expert/master
mode = classic

# Evil biome: RANDOM/corruption/crimson
evil = RANDOM

# Add a starter home at the spawn point.
home = false

# For a balanced world keep all the following settings at defaults.
#
# Unless mentioned otherwise on a specific option, all settings can be
# mixed together. Note that if an impossible condition is requested, for
# example more clouds than can fit in the sky, Terra AWG will make a best
# effort attempt anyway, but some features may fail to generate, and in
# rare cases, Terra AWG may freeze or crash (please report this so I can
# fix it).
[variation]
# Add starter equipment at the spawn point. Options:
#   none/iron/platinum/hellstone/mythril
equipment = none

# Select spawn point. "normal" spawn automatically determines location
# based on active secret seeds.
# "ashen" spawn increases the chances (but does not guarantee) of
# spawning in surface ashen fields. Options:
#   normal/surface/ashen/cloud/ocean/cavern/underworld
spawn = normal

# Prevent other settings from fully painting large structures and biomes.
unpainted = false

# Apply echo coating to a portion of the world. 0.5 means apply to half
# the tiles, 1.0 means apply to everything.
fadedMemories = 0.0

# Include both evil biomes and all ore variants.
# Activates "drunk world" secret seed.
doubleTrouble = false

# Break up the world into floating islands.
shattered = false

# Flood the majority of the world surface.
#
# Note that this may prevent meteorites. Also, Terraria's liquid engine
# has poor performance when settling large bodies of water.
sunken = false

# Only generate non-spreading biomes.
purity = false

# Start with hardmode activated.
hardmode = false

# Modify chest loot to be more applicable to a hardmode start.
hardmodeLoot = false

# Biome distribution layout. Options:
#   columns/layers/patches
biomes = columns

# Tuning options for biome patches. Reasonable values
# are between -0.5 and 0.5.
patchesHumidity = 0.0
patchesTemperature = 0.0

# How rapidly to transition biomes for biome patches.
patchesSize = 1.0

# Activates "celebrationmk10" secret seed.
celebration = false

# Activates "not the bees" secret seed.
hiveQueen = false

# Activates "for the worthy" secret seed.
forTheWorthy = false

# Activates "dont dig up" secret seed.
dontDigUp = false

# Number of meteorites to place.
meteorites = 0
meteoriteSize = 1.0

# Prevent corruption/crimson tendrils from extending into the sky.
trimEvilTendrils = false

# Reduce corruption/crimson overlap with major biomes.
avoidantEvil = false

# Placement frequency multipliers. 0.5 means half the
# normal amount, 2.0 means double the normal amount.
ore = 1.0
lifeCrystals = 1.0
manaCrystals = 1.0
pots = 1.0
chests = 1.0
gems = 1.0
# Activates "no traps" secret seed when greater than 15.
traps = 1.0
trees = 1.0
livingTrees = 1.0
clouds = 1.0
asteroids = 1.0
minecartTracks = 1.0
minecartLength = 1.0
aetherSize = 1.0
dungeonSize = 1.0
templeSize = 1.0
evilSize = 1.0
oceanSize = 1.0
oceanCaveSize = 1.0
marbleFreq = 1.0
marbleSize = 1.0
graniteFreq = 1.0
graniteSize = 1.0
glowingMushroomFreq = 1.0
glowingMushroomSize = 1.0
hiveFreq = 1.0
hiveSize = 1.0
spiderNestFreq = 1.0
spiderNestSize = 1.0
glowingMossFreq = 1.0
glowingMossSize = 1.0
# Snow, desert, and jungle size are ignored for biome patches.
snowSize = 1.0
desertSize = 1.0
jungleSize = 1.0
# Controls how flat/mountainous to generate surface terrain.
surfaceAmplitude = 1.0

[extra]
# Output a map preview image.
map = true
)";

// clang-format off
inline const std::vector adjectives{
    "Abandoned", "Abhorrent", "Adorable", "Adventurous", "Ageless",
    "Aggravating", "Aggressive", "Agile", "Agreeable", "Alert", "Alien",
    "Alive", "Alleged", "Aloof", "Amber", "Amethyst", "Amusing", "Ancient",
    "Angelic", "Angry", "Annoyed", "Annoying", "Anxious", "Archaic", "Ardent",
    "Arrogant", "Ashamed", "Attractive", "Average", "Awful", "Awkward",
    "Babbling", "Bad", "Baleful", "Bashful", "Basic", "Beautiful", "Best",
    "Bewildered", "Big", "Bitter", "Bizarre", "Black", "Blackish", "Bleeding",
    "Blistering", "Blocky", "Bloody", "Blooming", "Bloopy", "Blue", "Blushful",
    "Blushing", "Boorish", "Bored", "Boundless", "Brainy", "Brash", "Brave",
    "Breakable", "Breakless", "Breathtaking", "Bright", "Brilliant", "Broken",
    "Brown", "Brutal", "Budding", "Burning", "Busy", "Calm", "Canceled",
    "Careful", "Cautious", "Celestial", "Charming", "Chartreuse", "Cheap",
    "Cheerful", "Cherry", "Chicken", "Clean", "Clear", "Clearable", "Clever",
    "Cleverish", "Cloudy", "Clumsy", "Cold", "Colorful", "Combative",
    "Comfortable", "Comical", "Common", "Complacent", "Compulsive",
    "Concerned", "Condemnable", "Condemned", "Confident", "Confusable",
    "Confused", "Constant", "Cooked", "Cooperative", "Corpulent", "Corrupt",
    "Corrupted", "Cosmic", "Courageous", "Cranky", "Crazy", "Creepy", "Cringe",
    "Crowded", "Crude", "Cruel", "Cuddly", "Curious", "Cursed", "Cute", "Cyan",
    "Daft", "Damaged", "Dangerous", "Dank", "Dapper", "Dark", "Daunting",
    "Dead", "Decaying", "Deceased", "Defeated", "Defiant", "Degenerative",
    "Delightful", "Demented", "Demonic", "Dense", "Depressed", "Deranged",
    "Desolate", "Desperate", "Determined", "Devious", "Diamond", "Different",
    "Difficult", "Dire", "Dirty", "Disappointing", "Discarded", "Disgusting",
    "Disloyal", "Disruptive", "Distant", "Distinct", "Distorted",
    "Distressing", "Disturbed", "Divine", "Dizzy", "Docile", "Dope",
    "Doubtful", "Drab", "Draconian", "Dreadful", "Dreamy", "Dripping", "Dry",
    "Dubious", "Dull", "Eager", "Easy", "Eerie", "Elated", "Elegant",
    "Embarrassed", "Embarrassing", "Emerald", "Empty", "Enchanting",
    "Encouraging", "Enduring", "Energetic", "Enthusiastic", "Envious",
    "Erratic", "Eternal", "Euphoric", "Everlasting", "Evil", "Exalted",
    "Excellent", "Excited", "Exiled", "Existential", "Exotic", "Expensive",
    "Extinct", "Extra", "Extraordinary", "Extravagant", "Exuberant",
    "Fabulous", "Fair", "Faithful", "Fallen", "Famous", "Fancy", "Fantastic",
    "Far", "Faraway", "Fearful", "Fearsome", "Feckless", "Feral", "Fertile",
    "Festering", "Fetid", "Fierce", "Filthy", "Fine", "Firm", "Flaky",
    "Flaming", "Flat", "Flexing", "Flowering", "Flowing", "Fluffy",
    "Foolhardy", "Foolish", "Forceful", "Foreign", "Forgiving", "Forgotten",
    "Forsaken", "Fortified", "Foul", "Fragile", "Frail", "Frantic",
    "Fraudulent", "Fresh", "Friendly", "Frightened", "Frightening", "Frisky",
    "Fruitful", "Funny", "Furious", "Gangrenous", "Gentle", "Ghosted",
    "Gifted", "Glamorous", "Gleaming", "Gleamless", "Gloomy", "Glorious",
    "Glowing", "Goblin", "Godly", "Golden", "Good", "Gorgeous", "Graceful",
    "Grand", "Grassy", "Gray", "Greasy", "Great", "Greedy", "Green",
    "Griefing", "Grieving", "Grizzly", "Grotesque", "Grouchy", "Growing",
    "Gruesome", "Grumpy", "Guilty", "Gutless", "Hallowed", "Handsome", "Happy",
    "Hardcore", "Harsh", "Hateful", "Hazardous", "Healthy", "Heartless",
    "Heavenly", "Heinous", "Helpful", "Helpless", "Hesitant", "Hidden",
    "Hideous", "Highkey", "Hilarious", "Holy", "Homeless", "Homely",
    "Horrible", "Horrific", "Horrifying", "Hot", "Hungry", "Hurt", "Hurtable",
    "Hurtful", "Hybrid", "Hyper", "Hysterical", "Ignorant", "Illicit",
    "Illusive", "Imaginary", "Immortal", "Incompetent", "Inconvenient",
    "Indecisive", "Indifferent", "Infected", "Inferior", "Infinite",
    "Insecure", "Insidious", "Insolent", "Intense", "Irresponsible",
    "Irritating", "Isolated", "Itchy", "Jade", "Jealous", "Jiggly", "Jittery",
    "Jolly", "Joyous", "Judgmental", "Keen", "Kind", "Kooky", "Lagging",
    "Large", "Lasting", "Lavender", "Lavish", "Lazy", "Leafy", "Legendary",
    "Lemon", "Light", "Lightful", "Lime", "Little Known", "Lively", "Lonely",
    "Long", "Lost", "Lousy", "Lovely", "Lowkey", "Loyal", "Lucky", "Luminous",
    "Lumpy", "Lush", "Mad", "Magical", "Magnetic", "Magnificent",
    "Marshmallow", "Marvelous", "Massive", "Mathematical", "Menacing",
    "Merciless", "Mischievous", "Miserable", "Misty", "Modern", "Modular",
    "Moist", "Moldy", "Moonlit", "Mopey", "Motionless", "Mourning", "Muddy",
    "Multitalented", "Murderous", "Murky", "Mushy", "Mysterious", "Mythical",
    "Naive", "Nameless", "Nasty", "Natchy", "Natural", "Naughty", "Nauseating",
    "Navy", "Neglected", "Nervous", "New", "Nice", "Nimble", "Noxious",
    "Nutty", "Obedient", "Obnoxious", "Obsessive", "Obstructed", "Obtuse",
    "Odd", "Offended", "Offensive", "Old", "Old-fashioned", "Olive", "Open",
    "Optimistic", "Orange", "Outraged", "Outrageous", "Outstanding",
    "Overcrowded", "Overeasy", "Overjealous", "Overjoyous", "Overrun", "Pale",
    "Panicky", "Partying", "Patronizing", "Peaceful", "Peach", "Perfect",
    "Perilous", "Perpetual", "Persistent", "Petrified", "Pine", "Pink",
    "Placid", "Plain", "Planking", "Pleasant", "Pleasing", "Pointy", "Poised",
    "Poison", "Poor", "Posturing", "Powerful", "Precious", "Prickly",
    "Prismatic", "Proud", "Pumpkin", "Purified", "Purple", "Putrid", "Puzzled",
    "Quaint", "Quick", "Quiet", "Quirky", "Rancid", "Rank", "Raunchy", "Raw",
    "Real", "Reckless", "Red", "Reeking", "Rejected", "Relentless",
    "Relievable", "Relieved", "Remote", "Repugnant", "Repulsive", "Resentful",
    "Restful", "Revolting", "Rich", "Ridiculous", "Risky", "Rotten", "Rotting",
    "Round", "Royal", "Ruby", "Rude", "Ruthless", "Sacred", "Sad", "Safe",
    "Sage", "Sallow", "Salty", "Sandy", "Sapphire", "Saucy", "Savage",
    "Scandalous", "Scary", "Searing", "Seasick", "Sedated", "Selfish",
    "Senile", "Senseless", "Sensitive", "Serene", "Serious", "Shabby",
    "Shameful", "Sharp", "Shiny", "Shocking", "Shoddy", "Shy", "Significant",
    "Silenced", "Silly", "Silver", "Simple", "Skeleton", "Sleepy", "Slow",
    "Sluggish", "Smelly", "Smiling", "Smoggy", "Smol", "Smooth", "Smouldering",
    "Solar", "Solid", "Sore", "Sour", "Sparkling", "Spicy", "Spiritual",
    "Splendid", "Spoiled", "Spooky", "Spotless", "Stable", "Stalwart",
    "Staunch", "Steaming", "Sticky", "Stiff", "Still", "Stinky", "Stormy",
    "Strange", "Strong", "Subaverage", "Subpar", "Successful", "Super",
    "Superb", "Superior", "Supernatural", "Supreme", "Sweaty", "Tainted",
    "Talented", "Tame", "Tasty", "Teal", "Tenacious", "Tender", "Tense",
    "Terrible", "Terrified", "Thankful", "Thick", "Thoughtful", "Thoughtless",
    "Timeless", "Tiny", "Tired", "Topaz", "Tough", "Tranquil", "Trashy",
    "Traveling", "Tropical", "Troubled", "Trusting", "Ugliest", "Ugly",
    "Unacceptable", "Unbreakable", "Unbroken", "Uncanny", "Uncharming",
    "Uncharted", "Uncombative", "Uncooperative", "Undead", "Undetermined",
    "Undiscovered", "Undying", "Unending", "Unexplored", "Unforgivable",
    "Unhappy", "Unhurt", "Unhurting", "Uninterested", "Unknown", "Unlawful",
    "Unpleasant", "Unreal", "Unruly", "Unsightly", "Unsure", "Untalented",
    "Untamed", "Unusual", "Unwicked", "Unworried", "Upset", "Upsetable",
    "Uptight", "Useless", "Vague", "Vain", "Vampiric", "Vanilla", "Vast",
    "Verdant", "Vibrant", "Vicious", "Victorious", "Vile", "Violent", "Violet",
    "Viridian", "Vivacious", "Volatile", "Wacky", "Wandering", "Wary", "Weak",
    "Weary", "Weepy", "Weird", "Wet", "White", "Wicked", "Wild", "Windy",
    "Wishful", "Withered", "Witless", "Witty", "Woke", "Wonderful", "Wondrous",
    "Worried", "Worrisome", "Wriggly", "Wrong", "Yellow", "Zany", "Zealous"
};

inline const std::vector locations{
    "Abode", "Abyss", "Accident", "Acres", "Afterworld", "Alcove", "Alley",
    "Apogee", "Arbor", "Archipelago", "Area", "Arena", "Artwork", "Asylum",
    "Backcountry", "Backwater", "Backwoods", "Badlands", "Bane", "Bank",
    "Barren", "Base", "Basin", "Bastion", "Bay", "Bayou", "Beach", "Bed",
    "Bedrock", "Beyond", "Biosphere", "Bluff", "Body", "Bog", "Boil",
    "Boondocks", "Boonies", "Border", "Bottoms", "Boundary", "Bowl", "Box",
    "Breach", "Brewery", "Brook", "Bubble", "Bundle", "Bunker", "Burrow",
    "Bush", "Butte", "Camp", "Canal", "Canopy", "Canvas", "Canyon", "Cape",
    "Carton", "Center", "Chance", "Chaparral", "Chasm", "Citadel", "City",
    "Clearing", "Climax", "Cloudland", "Coast", "Colony", "Commune",
    "Confluence", "Constant", "Continent", "Convention", "Core", "Cosmos",
    "Couch", "Country", "County", "Court", "Courtyard", "Cove", "Crater",
    "Creek", "Crest", "Croak", "Crossing", "Crossroads", "Crown", "Cult",
    "Dale", "Daybreak", "Daydream", "Defecation", "Delight", "Dell", "Delta",
    "Den", "Depths", "Desert", "Dimension", "Dirt", "Disease", "District",
    "Dollop", "Domain", "Dome", "Door", "Dream", "Dreamland", "Dreamworld",
    "Drip", "Dump", "Dune", "Dungeon", "Easement", "Eater", "Eclipse", "Edge",
    "Egg", "Elevation", "Empire", "Empyrean", "Enclosure", "End", "Entity",
    "Essence", "Estate", "Estuary", "Eternity", "Ether", "Everglade",
    "Existence", "Expanse", "Exterior", "Eye", "Fable", "Fabrication", "Faith",
    "Fantasia", "Farm", "Field", "Figment", "Finger", "Firmament", "Fjord",
    "Flapper", "Flatland", "Flats", "Flex", "Flower", "Fold", "Folly", "Foot",
    "Foothold", "Forest", "Forge", "Fort", "Fortress", "Foundation",
    "Fountain", "Fraternity", "Front", "Frontier", "Galaxy", "Garden", "Gaze",
    "Geyser", "Glacier", "Glade", "Globe", "Grange", "Grassland", "Grave",
    "Graveyard", "Grounds", "Grove", "Gulf", "Gully", "Gutter", "Hamlet",
    "Harbor", "Harvest", "Haven", "Head", "Heap", "Heart", "Heaven", "Hedge",
    "Heights", "Hideout", "Highland", "Hill", "Hilltop", "Hinterland", "Hive",
    "Hole", "Hollow", "Homeland", "Honey", "Hope", "Horizon", "Hovel", "Hub",
    "Illusion", "Image", "Infection", "Interior", "Island", "Isle", "Islet",
    "Jalopy", "Jungle", "Keystone", "Kingdom", "Knoll", "Labor", "Labyrinth",
    "Lagoon", "Lair", "Lake", "Land", "Latte", "Leaf", "Legend", "Legs", "Lie",
    "Limbo", "Lining", "Locale", "Loch", "Magic", "Marsh", "Marshland", "Mass",
    "Maze", "Meadow", "Meridian", "Mesa", "Miasma", "Midland", "Mine",
    "Mirage", "Mire", "Mistake", "Mood", "Moon", "Moorland", "Morass",
    "Mortuary", "Moss", "Mound", "Mountain", "Mouth", "Myth", "Nation",
    "Neighborhood", "Nest", "Niche", "Nightmare", "Nirvana", "Nugget",
    "Nursery", "Oasis", "Object", "Ocean", "Orchard", "Origin", "Outback",
    "Outland", "Outskirts", "Overworld", "Paintbrush", "Parable", "Paradise",
    "Park", "Passage", "Passenger", "Passing", "Pasture", "Patch", "Peak",
    "Pearl", "Pedestal", "Peninsula", "Picture", "Pie", "Pile", "Pinnacle",
    "Pit", "Place", "Plains", "Planet", "Plateau", "Plaza", "Plot", "Plumbus",
    "Point", "Polestar", "Pond", "Port", "Portrait", "Pothole", "Prairie",
    "Prison", "Province", "Pub", "Quagmire", "Quarantine", "Ranch", "Rapids",
    "Ravine", "Reach", "Reality", "Realm", "Reef", "Refuge", "Region",
    "Regret", "Remotes", "Residence", "Rest", "Retreat", "Ridge", "Rift",
    "Ring", "River", "Roost", "Root", "Route", "Run", "Sack", "Salt",
    "Sanctuary", "Savanna", "Scrubland", "Sea", "Seaside", "Section", "Sector",
    "Settlement", "Shallows", "Shanty", "Shantytown", "Sheet", "Shire", "Shoe",
    "Shore", "Shrine", "Shroud", "Shrubbery", "Shrublands", "Site", "Sky",
    "Slice", "Slime", "Slope", "Slumber", "Snack", "Snap", "Sock", "Soil",
    "Soup", "Source", "Space", "Span", "Speedrun", "Sphere", "Spiral",
    "Spring", "Square", "State", "Station", "Steppe", "Stick", "Sticks",
    "Story", "Strait", "Stream", "Stretch", "Study", "Suburb", "Summit",
    "Sunrise", "Sunset", "Swamp", "Sweater", "Sweep", "Table", "Taiga", "Tale",
    "Tears", "Temple", "Terrace", "Terrain", "Terraria", "Terrarium",
    "Territory", "Thicket", "Throat", "Throne", "Tilt", "Timberland", "Tip",
    "Token", "Tomb", "Torch", "Touch", "Towel", "Town", "Tract", "Trail",
    "Trap", "Treasure", "Tree", "Trench", "Triangle", "Tributary", "Tropic",
    "Tundra", "Tunnel", "Turf", "Underbelly", "Undergrowth", "Underwear",
    "Underwood", "Universe", "Unknown", "Upland", "Utopia", "Vale", "Valley",
    "Vault", "Vibe", "Vicinity", "Vineyard", "Virus", "Vision", "Void", "Wall",
    "Ward", "Waste", "Wasteland", "Web", "Well", "Wetland", "Wharf",
    "Wilderness", "Wilds", "Wildwood", "Wonderland", "Wood", "Woodland",
    "Woods", "World", "Yard", "Zone"
};

inline const std::vector nouns{
    "Ability", "Absurdity", "Accidents", "Acorns", "Adamantite", "Adoration",
    "Advantage", "Adventure", "Agony", "Alarm", "Allergies", "Amazement",
    "Angels", "Anger", "Anguish", "Annoyance", "Anvils", "Anxiety", "Apples",
    "Apricots", "Argon", "Arrows", "Arthritis", "Asbestos", "Ash", "Assassins",
    "Atrophy", "Awareness", "Awe", "Bacon", "Balance", "Balloons", "Bamboo",
    "Bananas", "Bandits", "Bankruptcy", "Bark", "Bats", "Beauty", "Beenades",
    "Bees", "Beggars", "Belief", "Betrayers", "Birds", "Birthdays",
    "Bitterness", "Blasphemy", "Blindness", "Blinkroot", "Blocks", "Blood",
    "Bloodletting", "Bloodshed", "Blossoms", "Bodies", "Bone", "Bones",
    "Boomers", "Boots", "Boredom", "Boulders", "Brains", "Branches", "Bravery",
    "Bribery", "Bridges", "Brilliance", "Bubbles", "Buckets", "Bugs",
    "Bunnies", "Burglars", "Burnination", "Buttercups", "Butterflies", "Cacti",
    "Calmness", "Candy", "Care", "Carnage", "Carrion", "Casualty", "Cats",
    "Cattails", "Caves", "Celebration", "Chainsaws", "Change", "Chaos",
    "Charity", "Cheats", "Cherries", "Chests", "Childhood", "Children",
    "Chlorophyte", "Cilantro", "Clarity", "Clay", "Clentamination",
    "Cleverness", "Clouds", "Cobalt", "Coconuts", "Coffee", "Coins",
    "Coldness", "Comfort", "Compassion", "Compost", "Concern", "Confidence",
    "Confinement", "Confusion", "Contentment", "Convicts", "Copper", "Corpses",
    "Corruption", "Courage", "Creation", "Creatures", "Creepers", "Crests",
    "Crime", "Criminals", "Crimtane", "Crooks", "Crows", "Crystals", "Cthulhu",
    "Curiosity", "Cyborgs", "Daisies", "Darkness", "Darts", "Daughters",
    "Dawn", "Daybloom", "Deadbeats", "Death", "Deathweed", "Debt", "Deceit",
    "Deception", "Dedication", "Defeat", "Degradation", "Delay", "Delusion",
    "Demonite", "Demons", "Derangement", "Despair", "Desperation",
    "Destruction", "Dirt", "Disappointment", "Disgust", "Dishonesty", "Dismay",
    "Distortion", "Distribution", "Dogs", "Doom", "Dragonfruit", "Dragons",
    "Dread", "Dreams", "Drills", "Drums", "Ducks", "Dusk", "Dust", "Duty",
    "Dysphoria", "Ears", "Education", "Eggs", "Elderberries", "Elegance",
    "Envy", "Evasion", "Evil", "Exile", "Exploits", "Explosives", "Extortion",
    "Eyes", "Fable", "Facts", "Failures", "Fairies", "Faith", "Falsehood",
    "Fame", "Famine", "Fantasy", "Fascination", "Fatality", "Fear", "Feathers",
    "Felons", "Ferns", "Fiction", "Fiends", "Fingers", "Fireblossom",
    "Fireflies", "Fish", "Flails", "Flatulence", "Flatus", "Flesh", "Floof",
    "Flowers", "Foam", "Forgery", "Forgiveness", "Forks", "Fortitude", "Fraud",
    "Freaks", "Freckles", "Freedom", "Friendship", "Fright", "Frogs", "Frost",
    "Fruit", "Gangsters", "Garbage", "Gears", "Gel", "Gemcorns", "Generation",
    "Ghosts", "Giggles", "Gingers", "Glass", "Gloom", "Gluttony", "Goals",
    "Goblins", "Gold", "Goldfish", "Gossip", "Grain", "Granite", "Grapefruit",
    "Grapes", "Grass", "Grasshoppers", "Graves", "Greed", "Grief", "Griefers",
    "Guitars", "Guts", "Hair", "Hamburgers", "Hammers", "Hands", "Happiness",
    "Hardship", "Harmony", "Harpies", "Hate", "Hatred", "Heart", "Heartache",
    "Hearts", "Heels", "Hellstone", "Herbs", "Heresy", "Hoiks", "Honey",
    "Hoodlums", "Hooks", "Hooligans", "Hope", "Hopelessness", "Hornets",
    "Horns", "Hornswoggle", "Horror", "Horrors", "Houses", "Humanity",
    "Humiliation", "Hurt", "Hysteria", "Ice", "Ichor", "Illness",
    "Indictments", "Indigestion", "Indignity", "Infancy", "Infections",
    "Inflammation", "Inflation", "Injury", "Insanity", "Insects",
    "Intelligence", "Intestines", "Invasions", "Iron", "Irritation",
    "Isolation", "Ivy", "Jaws", "Jealousy", "Jellyfish", "Joy", "Justice",
    "Kidneys", "Kindness", "Kittens", "Knives", "Krypton", "Lamps", "Larceny",
    "Laughter", "Lava", "Lawsuits", "Lawyers", "Lead", "Learning", "Leaves",
    "Legend", "Legends", "Leggings", "Legs", "Lemons", "Letdown", "Lethargy",
    "Liberty", "Lies", "Life", "Lilies", "Lilith", "Lilypads", "Lips",
    "Listening", "Litigation", "Livers", "Loathing", "Lombago", "Loneliness",
    "Loot", "Lore", "Losers", "Loss", "Love", "Luck", "Luggage", "Luminite",
    "Lungs", "Luxury", "Lyrics", "Madness", "Maggots", "Man", "Mana", "Mangos",
    "Mania", "Mankind", "Manslaughter", "Marble", "Markets", "Marvel",
    "Mastication", "Maturity", "Medicine", "Melancholy", "Melodies", "Memes",
    "Mercy", "Meteorite", "Mice", "Midnight", "Milk", "Mimics", "Miracles",
    "Mirrors", "Misery", "Misfortune", "Models", "Money", "Monotony",
    "Moonglow", "Moonlight", "Morons", "Mortality", "Moss", "Mourning",
    "Mouths", "Movement", "Mucus", "Mud", "Mushrooms", "Music", "Mystery",
    "Myth", "Mythril", "Nausea", "Necromancers", "Necromancy", "Night",
    "Nightcrawlers", "Nightmares", "Nostalgia", "Obscurity", "Obsidian",
    "Odor", "Ogres", "Ooze", "Opportunity", "Options", "Oranges", "Orchids",
    "Organs", "Orichalcum", "Outlaws", "Owls", "Pain", "Palladium",
    "Panhandlers", "Panic", "Pansies", "Parasites", "Parties", "Partying",
    "Patience", "Peace", "Penguins", "Peril", "Perjury", "Perspiration",
    "Pickaxes", "Pickpockets", "Pineapples", "Pinky", "Piranha", "Piranhas",
    "Pirates", "Pixies", "Pizza", "Plantero", "Plants", "Platinum", "Pleasure",
    "Plums", "Politicians", "Ponies", "Potions", "Poverty", "Power", "Pride",
    "Prisms", "Privacy", "Promises", "Prophecy", "Psychology", "Puppies",
    "Purity", "Rain", "Rainbows", "Ramen", "Rats", "Reality", "Redemption",
    "Regret", "Regurgitation", "Relaxation", "Relief", "Remorse", "Repugnance",
    "Riches", "Rocks", "Rope", "Roses", "Ruination", "Rumours", "Sacrifice",
    "Sacrilege", "Sadness", "Salesmen", "Sand", "Sandstone", "Sanity", "Sap",
    "Saplings", "Sashimi", "Satisfaction", "Sauce", "Scandal", "Scorpions",
    "Screams", "Seasons", "Seaweed", "Seclusion", "Secrecy", "Secrets",
    "Seeds", "Services", "Shade", "Shadows", "Shenanigans", "Shivers",
    "Shiverthorn", "Shock", "Shrimp", "Silliness", "Silt", "Silver", "Sin",
    "Skeletons", "Skill", "Skin", "Skulls", "Sleep", "Slime", "Sloth",
    "Sloths", "Smiles", "Smoke", "Snails", "Snakes", "Snatchers", "Snow",
    "Solicitation", "Songs", "Sorrow", "Souls", "Sounds", "Spaghetti",
    "Sparkles", "Spears", "Speed", "Spikes", "Spirits", "Splinters", "Sponges",
    "Sprinkles", "Spurs", "Squid", "Squirrels", "Starfruit", "Starvation",
    "Statues", "Stone", "Strength", "Stress", "Strictness", "Stumps",
    "Suffering", "Sunflowers", "Sunshine", "Superstition", "Surprise",
    "Swagger", "Swindlers", "Swords", "Talent", "Taxation", "Taxes",
    "Teamwork", "Teeth", "Terror", "the Ancients", "the Angler", "the Apple",
    "the Archer", "the Aunt", "the Axe", "the Baby", "the Ball", "the Balloon",
    "the Bat", "the Beast", "the Betrayed", "the Blender", "the Blood Moon",
    "the Bow", "the Bride", "the Bubble", "the Bunny", "the Cactus",
    "the Cloud", "the Corruptor", "the Crab", "the Dance", "the Dark",
    "the Dead", "the Devourer", "the Drax", "the Ducks", "the Eclipse",
    "the Elderly", "the Fairy", "the Father", "the Fool", "the Foot",
    "the Frozen", "the Gift", "the Ginger", "the Goat", "the Goblin",
    "the Golem", "the Groom", "the Guest", "the Hammer", "the Hammush",
    "the Head", "the Heavens", "the Hipster", "the Hobo", "the Homeless",
    "the King", "the Law", "the Library", "the Lihzahrd", "the Lilith",
    "the Lizard King", "the Lost Generation", "the Manager", "the Merchant",
    "the Mirror", "the Monster", "the Moon", "the Mother", "the Mummy",
    "the Mushroom", "the Needy", "the Old One", "the Pandemic", "the Pickaxe",
    "the Picksaw", "the Pigron", "the Porcelain God", "the Princess",
    "the Prism", "the Prodigy", "the Prophecy", "the Pwnhammer", "the Queen",
    "the Ramen", "the Right", "the Scholar", "the Shark", "the Sickle",
    "the Sky", "the Snap", "the Snitch", "the Spelunker", "the Spirits",
    "the Staff", "the Stars", "the Stench", "the Stooge", "the Sun",
    "the Sword", "the Tooth", "The Torch God", "the Tortoise", "the Tree",
    "the Trend", "the Undead", "the Unicorn", "the Union", "the Unknown",
    "the Varmint", "the Waraxe", "the Wise", "the World", "the Yoyo",
    "the Zoologist", "Thieves", "Thinking", "Thorns", "Thunder", "Time", "Tin",
    "Tingling", "Tiredness", "Titanium", "Tombstones", "Torches", "Torment",
    "Traitors", "Tranquility", "Traps", "Trash", "Treasure", "Trees", "Trends",
    "Trouble", "Truffles", "Trunks", "Trust", "Tulips", "Tungsten", "Twigs",
    "Twilight", "Umbrellas", "Understanding", "Vagabonds", "Vampires",
    "Vanity", "Venom", "Victims", "Victory", "Villains", "Vines", "Violets",
    "Vomit", "Vultures", "Wands", "Wariness", "Warmth", "Wasps", "Waterleaf",
    "Weakness", "Wealth", "Webs", "Weeds", "Werewolves", "Whoopsies", "Wings",
    "Winners", "Winning", "Wires", "Wisdom", "Wizards", "Woe", "Wolves",
    "Wonder", "Wood", "Worlds", "Worms", "Worries", "Wrath", "Wrenches",
    "Wyverns", "Xenon", "Yoyos", "Zombies"
};
// clang-format on

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

EvilBiome parseEvilBiome(const std::string &evil)
{
    if (evil == "corruption") {
        return EvilBiome::corruption;
    } else if (evil == "crimson") {
        return EvilBiome::crimson;
    } else if (evil != "RANDOM") {
        std::cout << "Unknown evil '" << evil << "'\n";
    }
    return EvilBiome::random;
}

int parseEquipment(const std::string &equipment)
{
    if (equipment == "iron") {
        return ItemID::ironBar;
    } else if (equipment == "platinum") {
        return ItemID::platinumBar;
    } else if (equipment == "hellstone") {
        return ItemID::hellstoneBar;
    } else if (equipment == "mythril") {
        return ItemID::mythrilBar;
    } else if (equipment == "debug") {
        return ItemID::zenith;
    } else if (equipment != "none") {
        std::cout << "Unknown equipment '" << equipment << "'\n";
    }
    return 0;
}

SpawnPoint parseSpawn(const std::string &spawn)
{
    if (spawn == "surface") {
        return SpawnPoint::surface;
    } else if (spawn == "ashen") {
        return SpawnPoint::ashen;
    } else if (spawn == "cloud") {
        return SpawnPoint::cloud;
    } else if (spawn == "ocean") {
        return SpawnPoint::ocean;
    } else if (spawn == "cavern") {
        return SpawnPoint::cavern;
    } else if (spawn == "underworld") {
        return SpawnPoint::underworld;
    } else if (spawn != "normal") {
        std::cout << "Unknown spawn '" << spawn << "'\n";
    }
    return SpawnPoint::normal;
}

BiomeLayout parseBiomeLayout(const std::string &biomes)
{
    if (biomes == "layers") {
        return BiomeLayout::layers;
    } else if (biomes == "patches") {
        return BiomeLayout::patches;
    } else if (biomes != "columns") {
        std::cout << "Unknown biome layout '" << biomes << "'\n";
    }
    return BiomeLayout::columns;
}

std::string genRandomName(Random &rnd)
{
    switch (rnd.getInt(0, 7)) {
    default:
    case 0:
    case 1:
        return std::string{"The "} + rnd.select(adjectives) + " " +
               rnd.select(locations) + " of " + rnd.select(nouns);
    case 2:
    case 3:
        return std::string{} + rnd.select(adjectives) + " " +
               rnd.select(locations) + " of " + rnd.select(nouns);
    case 4:
        return std::string{"The "} + rnd.select(adjectives) + " " +
               rnd.select(locations);
    case 5:
        return std::string{} + rnd.select(adjectives) + " " +
               rnd.select(locations);
    case 6:
        return std::string{"The "} + rnd.select(locations) + " of " +
               rnd.select(nouns);
    case 7:
        return std::string{} + rnd.select(locations) + " of " +
               rnd.select(nouns);
    }
}

std::string processSeed(const std::string &baseSeed, Random &rnd)
{
    if (baseSeed == "RANDOM" || baseSeed.empty()) {
        return "AWG-" + std::to_string(
                            rnd.getInt(0, std::numeric_limits<int32_t>::max()));
    }
    if (baseSeed.size() < 7) {
        return baseSeed;
    }
    for (int i = 0; i < 3; ++i) {
        if (!std::isdigit(baseSeed[2 * i]) || baseSeed[2 * i + 1] != '.') {
            return baseSeed;
        }
    }
    return baseSeed.substr(6);
}

std::string Config::getFilename() const
{
    std::string filename(name);
    for (char &c : filename) {
        if (!std::isalnum(c)) {
            c = '_';
        }
    }
    return filename;
}

#define READ_CONF_VALUE(SECTION, KEY, TYPE)                                    \
    conf.KEY = reader.Get##TYPE(#SECTION, #KEY, conf.KEY)

#define READ_CONF_AREA_VALUE(SECTION, KEY)                                     \
    conf.KEY =                                                                 \
        std::sqrt(std::max(reader.GetReal(#SECTION, #KEY, conf.KEY), 0.0))

Config readConfig(Random &rnd)
{
    Config conf{
        "Terra AWG World",
        "RANDOM",
        6400,
        1800,
        GameMode::classic,
        EvilBiome::random,
        false, // starterHome
        0,     // equipment
        SpawnPoint::normal,
        false, // unpainted
        0.0,   // fadedMemories
        false, // doubleTrouble
        false, // shattered
        false, // sunken
        false, // purity
        false, // hardmode
        false, // hardmodeLoot
        BiomeLayout::columns,
        0.0,   // patchesHumidity
        0.0,   // patchesTemperature
        1.0,   // patchesSize
        false, // celebration
        false, // hiveQueen
        false, // forTheWorthy
        false, // dontDigUp
        0,     // meteorites
        1.0,   // meteoriteSize
        false, // trimEvilTendrils
        false, // avoidantEvil
        1.0,   // ore
        1.0,   // lifeCrystals
        1.0,   // manaCrystals
        1.0,   // pots
        1.0,   // chests
        1.0,   // gems
        1.0,   // traps
        1.0,   // trees
        1.0,   // livingTrees
        1.0,   // clouds
        1.0,   // asteroids
        1.0,   // minecartTracks
        1.0,   // minecartLength
        1.0,   // aetherSize
        1.0,   // dungeonSize
        1.0,   // templeSize
        1.0,   // evilSize
        1.0,   // oceanSize
        1.0,   // oceanCaveSize
        1.0,   // marbleFreq
        1.0,   // marbleSize
        1.0,   // graniteFreq
        1.0,   // graniteSize
        1.0,   // glowingMushroomFreq
        1.0,   // glowingMushroomSize
        1.0,   // hiveFreq
        1.0,   // hiveSize
        1.0,   // spiderNestFreq
        1.0,   // spiderNestSize
        1.0,   // glowingMossFreq
        1.0,   // glowingMossSize
        1.0,   // snowSize
        1.0,   // desertSize
        1.0,   // jungleSize
        1.0,   // surfaceAmplitude
        true}; // map
    if (!std::filesystem::exists(confName)) {
        std::ofstream out(confName, std::ios::out);
        out.write(defaultConfigStr, std::strlen(defaultConfigStr));
    }
    INIReader reader(confName);
    if (reader.ParseError() < 0) {
        std::cout << "Unable to load config from'" << confName << "'\n";
        conf.seed = processSeed(conf.seed, rnd);
        return conf;
    }
    conf.name = reader.Get("world", "name", conf.name);
    if (conf.name == "RANDOM") {
        conf.name = genRandomName(rnd);
    }
    conf.seed = processSeed(reader.Get("world", "seed", conf.seed), rnd);
    READ_CONF_VALUE(world, width, Integer);
    READ_CONF_VALUE(world, height, Integer);
    conf.mode = parseGameMode(reader.Get("world", "mode", "classic"));
    conf.evil = parseEvilBiome(reader.Get("world", "evil", "RANDOM"));
    READ_CONF_VALUE(world, home, Boolean);
    conf.equipment =
        parseEquipment(reader.Get("variation", "equipment", "none"));
    conf.spawn = parseSpawn(reader.Get("variation", "spawn", "normal"));
    READ_CONF_VALUE(variation, unpainted, Boolean);
    READ_CONF_VALUE(variation, fadedMemories, Real);
    READ_CONF_VALUE(variation, doubleTrouble, Boolean);
    READ_CONF_VALUE(variation, shattered, Boolean);
    READ_CONF_VALUE(variation, sunken, Boolean);
    READ_CONF_VALUE(variation, purity, Boolean);
    READ_CONF_VALUE(variation, hardmode, Boolean);
    READ_CONF_VALUE(variation, hardmodeLoot, Boolean);
    conf.biomes =
        parseBiomeLayout(reader.Get("variation", "biomes", "columns"));
    READ_CONF_VALUE(variation, patchesHumidity, Real);
    READ_CONF_VALUE(variation, patchesTemperature, Real);
    READ_CONF_AREA_VALUE(variation, patchesSize);
    READ_CONF_VALUE(variation, celebration, Boolean);
    READ_CONF_VALUE(variation, hiveQueen, Boolean);
    READ_CONF_VALUE(variation, forTheWorthy, Boolean);
    READ_CONF_VALUE(variation, dontDigUp, Boolean);
    READ_CONF_VALUE(variation, meteorites, Integer);
    READ_CONF_AREA_VALUE(variation, meteoriteSize);
    READ_CONF_VALUE(variation, trimEvilTendrils, Boolean);
    READ_CONF_VALUE(variation, avoidantEvil, Boolean);
    READ_CONF_VALUE(variation, ore, Real);
    READ_CONF_VALUE(variation, lifeCrystals, Real);
    READ_CONF_VALUE(variation, manaCrystals, Real);
    READ_CONF_VALUE(variation, pots, Real);
    READ_CONF_VALUE(variation, chests, Real);
    READ_CONF_VALUE(variation, gems, Real);
    READ_CONF_VALUE(variation, traps, Real);
    READ_CONF_VALUE(variation, trees, Real);
    READ_CONF_VALUE(variation, livingTrees, Real);
    READ_CONF_VALUE(variation, clouds, Real);
    READ_CONF_VALUE(variation, asteroids, Real);
    READ_CONF_VALUE(variation, minecartTracks, Real);
    READ_CONF_VALUE(variation, minecartLength, Real);
    READ_CONF_AREA_VALUE(variation, aetherSize);
    READ_CONF_VALUE(variation, dungeonSize, Real);
    READ_CONF_AREA_VALUE(variation, templeSize);
    READ_CONF_AREA_VALUE(variation, evilSize);
    READ_CONF_VALUE(variation, oceanSize, Real);
    READ_CONF_VALUE(variation, oceanCaveSize, Real);
    READ_CONF_VALUE(variation, marbleFreq, Real);
    READ_CONF_AREA_VALUE(variation, marbleSize);
    READ_CONF_VALUE(variation, graniteFreq, Real);
    READ_CONF_AREA_VALUE(variation, graniteSize);
    READ_CONF_VALUE(variation, glowingMushroomFreq, Real);
    READ_CONF_AREA_VALUE(variation, glowingMushroomSize);
    READ_CONF_VALUE(variation, hiveFreq, Real);
    READ_CONF_AREA_VALUE(variation, hiveSize);
    READ_CONF_VALUE(variation, spiderNestFreq, Real);
    READ_CONF_AREA_VALUE(variation, spiderNestSize);
    READ_CONF_VALUE(variation, glowingMossFreq, Real);
    READ_CONF_AREA_VALUE(variation, glowingMossSize);
    READ_CONF_VALUE(variation, snowSize, Real);
    READ_CONF_VALUE(variation, desertSize, Real);
    READ_CONF_VALUE(variation, jungleSize, Real);
    READ_CONF_VALUE(variation, surfaceAmplitude, Real);
    READ_CONF_VALUE(extra, map, Boolean);
    return conf;
}
