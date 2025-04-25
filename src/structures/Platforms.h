#ifndef PLATFORMS_H
#define PLATFORMS_H

class World;

namespace Platform
{
enum {
    metalShelf = 162,
    brassShelf = 180,
    woodShelf = 198,
    dungeonShelf = 216,
    obsidian = 234,
    lihzahrd = 594,
    stone = 774,
};
}

void placePlatform(int x, int y, int style, World &world);

#endif // PLATFORMS_H
