#ifndef TILE_H
#define TILE_H

enum class Liquid { none, water, lava, honey, shimmer };

enum class Slope { none = 0, half, topRight, topLeft, bottomRight, bottomLeft };

class Tile
{
public:
    int blockID;
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

    bool operator==(const Tile &) const = default;
};

#endif // TILE_H
