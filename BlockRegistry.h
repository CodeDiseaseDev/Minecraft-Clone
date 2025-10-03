//
// Created by code on 10/3/25.
//

#ifndef BLOCKREGISTRY_H
#define BLOCKREGISTRY_H
#include <unordered_map>

#include "Block.h"

enum FaceIndex {
  POS_X = 0,
  NEG_X = 1,
  POS_Y = 2,
  NEG_Y = 3,
  POS_Z = 4,
  NEG_Z = 5
};


struct BlockRegistry {
  static std::unordered_map<BlockID, BlockUV> BlockUVs;

  static void Init();
};



#endif //BLOCKREGISTRY_H
