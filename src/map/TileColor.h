#ifndef TILECOLOR_H
#define TILECOLOR_H

#include <cstdint>

class World;

class Color
{
public:
    Color(uint8_t *data);
    void blend(Color tint, double strength = 0.3);
    void hueBlend(Color tint);

    uint8_t rgb[3];
};

Color getTileColor(int x, int y, World &world);

#endif // TILECOLOR_H
