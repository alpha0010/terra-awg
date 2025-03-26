#ifndef CHEST_H
#define CHEST_H

#include "Item.h"
#include <array>

class Chest
{
public:
    int x;
    int y;
    std::array<Item, 40> items;
};

#endif // CHEST_H
