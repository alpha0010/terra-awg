#ifndef TREES_H
#define TREES_H

#include "TileBuffer.h"

namespace Data
{

enum class Tree {
    mushroom1,
    mushroom2,
    mushroom3,
    mushroom4,
    mushroom5,
    mushroom6,
    mushroom7,
    mushroom8,
    mushroom9,
    mushroom10,
    mushroom11,
    mushroom12,
    mushroom13,
    mushroom14,
    mushroom15,
    mushroom16,
};

inline std::array const mushroomTrees = {
    Tree::mushroom1,
    Tree::mushroom2,
    Tree::mushroom3,
    Tree::mushroom4,
    Tree::mushroom5,
    Tree::mushroom6,
    Tree::mushroom7,
    Tree::mushroom8,
    Tree::mushroom9,
    Tree::mushroom10,
    Tree::mushroom11,
    Tree::mushroom12,
    Tree::mushroom13,
    Tree::mushroom14,
    Tree::mushroom15,
    Tree::mushroom16,
};

TileBuffer getTree(Tree treeId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // TREES_H
