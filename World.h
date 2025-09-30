//
// Created by code on 9/23/25.
//

#ifndef WORLD_H
#define WORLD_H
#include <array>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Chunk.h"

#define WORLD_HEIGHT_CHUNKS (256 / 16) // 16 subchunks tall

struct ChunkColumn {
  std::array<std::shared_ptr<Chunk>, WORLD_HEIGHT_CHUNKS> chunks;
};

struct ChunkCoord {
  int x, y;
  bool operator==(const ChunkCoord& other) const noexcept {
    return x == other.x && y == other.y;
  }
};

struct ChunkCoordHash {
  std::size_t operator()(const ChunkCoord& c) const noexcept {
    // simple hash combine
    return (std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1));
  }
};


inline int floorDiv(int a, int b) {
  int q = a / b;
  int r = a % b;
  if ((r != 0) && ((r < 0) != (b < 0))) {
    --q;
  }
  return q;
}


inline int floorMod(int a, int b) {
  int r = a % b;
  if (r < 0) r += b;
  return r;
}

class World {
private:
  int seed = 0;

public:
  std::unordered_map<ChunkCoord, ChunkColumn, ChunkCoordHash> chunkColumns;

  float gravity = -30.0f;
  // float gravity = -0.2f;

  World(int seed) : seed(seed) {}

  Chunk& getChunkAt(int x, int y, int z);

  Block& getBlockAt(int x, int y, int z);
  void setBlockAt(int x, int y, int z, Block block);

  ChunkColumn &getOrCreateColumn(int cx, int cz);

  std::vector<std::shared_ptr<Chunk>> ensureChunkAndNeighbors(
    int worldX, int worldY, int worldZ, int radius);

  std::optional<glm::vec3> raycastBlock(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float maxDistance);

  std::vector<std::reference_wrapper<Block>> getNearbyBlocks(glm::vec3 vec);
};



#endif //WORLD_H
