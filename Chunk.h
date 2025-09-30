//
// Created by code on 9/23/25.
//

#ifndef CHUNK_H
#define CHUNK_H
#include <glm/vec3.hpp>

#include "Block.h"

static inline constexpr int CHUNK_SIZE = 16;

class Chunk {
public:

  glm::ivec3 position;

  Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

  Chunk(int chunkX, int chunkY, int chunkZ, int seed);

  bool isAir(int x, int y, int z) { return get(x, y, z).isAir(); }
  Block& get(int x, int y, int z) { return blocks[x][y][z]; }
  const Block& get(int x, int y, int z) const { return blocks[x][y][z]; }
  void set(int x, int y, int z, BlockID type) { blocks[x][y][z].id = type; }
};



#endif //CHUNK_H
