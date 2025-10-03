//
// Created by code on 10/3/25.
//

#include "BlockRegistry.h"

std::unordered_map<BlockID, BlockUV> BlockRegistry::BlockUVs;

void BlockRegistry::Init() {
  BlockUVs[BlockID::Grass] = BlockUV({{3,25}, {3,25}, {6,25}, {0,21}, {3,25}, {3,25}});
  BlockUVs[BlockID::Stone] = BlockUV({{14,52}, {14,52}, {14,52}, {14,52}, {14,52}, {14,52}});
  BlockUVs[BlockID::Dirt] = BlockUV({{0,21}, {0,21}, {0,21}, {0,21}, {0,21}, {0,21}});
  BlockUVs[BlockID::DiamondOre] = BlockUV({{9,19}, {9,19}, {9,19}, {9,19}, {9,19}, {9,19}});

}
