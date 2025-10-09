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

#include "arena_alloc.h"
#include "Chunk.h"

#define WORLD_HEIGHT_CHUNKS (100 / 16) // 16 subchunks tall

struct RaycastHit {
  glm::ivec3 voxel;   // block position
  glm::ivec3 normal;  // face normal of the block you hit

  [[nodiscard]] glm::ivec3 adjacent() const {
    return voxel + normal;
  }
};

class ChunkColumn {
public:

  std::array<Chunk*, WORLD_HEIGHT_CHUNKS> chunks;
  glm::vec3 colCenter;

  void deallocate(arena::Allocator<std::byte> arena) {
    for (auto& chunk : chunks) {
      chunk->~Chunk();
      AUTO_DEALLOCATE_CA(arena, Chunk, chunk);
    }
  }
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
  std::vector<Chunk*> pendingChunks;

  Chunk* makeChunk(int chunk_x, int chunk_y, int chunk_z);

  arena::Allocator<std::byte> &arena;

  std::vector<Chunk*> new_chunks;

public:
  std::unordered_map<ChunkCoord, ChunkColumn, ChunkCoordHash> chunkColumns;

  float gravity = -0.0f;
  // float gravity = -0.2f;

  World(arena::Allocator<std::byte> &arena, int seed) :
    seed(seed), arena(arena) {}

  Chunk& getChunkAt(int x, int y, int z);
  Chunk* getChunkPtrAt(int x, int y, int z);
  Chunk* tryGetChunkPtrAt(int x, int y, int z);


  Block& getBlockAt(int x, int y, int z);
  Block* tryGetBlockAt(int x, int y, int z);

  void setBlockAtAndUpdate(int x, int y, int z, Block block);
  void setBlockAt(int x, int y, int z, Block block);

  void setChunkDirty(int x, int y, int z);

  ChunkColumn &getOrCreateColumn(int cx, int cz);

  std::vector<Chunk*> ensureChunkAndNeighbors(
    int worldX, int worldY, int worldZ, int radius);

  std::optional<RaycastHit> raycastBlock(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float maxDistance);

  std::vector<std::reference_wrapper<Block>> getNearbyBlocks(glm::vec3 vec);

  bool isAir(int x, int y, int z);
  bool isAir(glm::ivec3 pos);



  ~World();
};



#endif //WORLD_H
