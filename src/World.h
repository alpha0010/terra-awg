#ifndef WORLD_H
#define WORLD_H

class World
{
private:
    int width;
    int height;

public:
    World();

    int getWidth();
    int getHeight();
    int getUndergroundLevel();
    int getCavernLevel();
};

#endif // WORLD_H
