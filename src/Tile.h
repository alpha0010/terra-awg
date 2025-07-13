#ifndef TILE_H
#define TILE_H

#include "ids/TileID.h"

enum class Liquid { none, water, lava, honey, shimmer };

// Described location is the portion of the block that will display as empty
// space.
enum class Slope { none = 0, half, topRight, topLeft, bottomRight, bottomLeft };

enum class Flag { none, border, hive, crispyHoney, yellow, orange };

class Tile
{
public:
    int blockID = TileID::empty;
    int frameX;
    int frameY;
    int wallID;
    int blockPaint;
    int wallPaint;
    Liquid liquid;
    Slope slope;
    bool wireRed : 1;
    bool wireBlue : 1;
    bool wireGreen : 1;
    bool wireYellow : 1;
    bool actuated : 1;
    bool actuator : 1;
    bool echoCoatBlock : 1;
    bool echoCoatWall : 1;
    bool illuminantBlock : 1;
    bool illuminantWall : 1;
    /**
     * Flag this tile for cooperative protection against overwrites/cleanup
     * changes.
     */
    bool guarded : 1;
    Flag flag;

    bool operator==(const Tile &) const = default;
};

#endif // TILE_H
