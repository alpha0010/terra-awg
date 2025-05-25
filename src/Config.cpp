#include "Config.h"

#include "Random.h"
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

# World size:
# - Small: 4200 x 1200
# - Medium: 6400 x 1800
# - Large: 8400 x 2400
# Other values may produce unexpected results.
width = 6400
height = 1800

# Difficulty: journey/classic/expert/master
mode = classic

# Add a starter home at the spawn point.
home = false

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

std::string Config::getFilename()
{
    std::string filename(name);
    for (char &c : filename) {
        if (!std::isalnum(c)) {
            c = '_';
        }
    }
    return filename;
}

Config readConfig(Random &rnd)
{
    Config conf{"Terra AWG World", 6400, 1800, GameMode::classic, false, true};
    if (!std::filesystem::exists(confName)) {
        std::ofstream out(confName, std::ios::out);
        out.write(defaultConfigStr, std::strlen(defaultConfigStr));
    }
    INIReader reader(confName);
    if (reader.ParseError() < 0) {
        std::cout << "Unable to load config from'" << confName << "'\n";
        return conf;
    }
    conf.name = reader.Get("world", "name", conf.name);
    if (conf.name == "RANDOM") {
        conf.name = genRandomName(rnd);
    }
    conf.width = reader.GetInteger("world", "width", conf.width);
    conf.height = reader.GetInteger("world", "height", conf.height);
    conf.mode = parseGameMode(reader.Get("world", "mode", "classic"));
    conf.starterHome = reader.GetBoolean("world", "home", conf.starterHome);
    conf.mapPreview = reader.GetBoolean("extra", "map", conf.mapPreview);
    return conf;
}
