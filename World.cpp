//
// Created by code on 9/23/25.
//

#include "World.h"

#include <cfloat>
#include <glm/common.hpp>
#include <iterator>

#include "Player.h"
// #include <glm/detail/func_geometric.inl>

Chunk & World::getChunkAt(int x, int y, int z) {
  return *getChunkPtrAt(x, y, z);
}

Chunk* World::makeChunk(int chunk_x, int chunk_y, int chunk_z) {
  return arena_allocate<Chunk>(arena, chunk_x, chunk_y, chunk_z, seed);
}

Chunk* World::getChunkPtrAt(int x, int y, int z) {
  int chunk_x = floorDiv(x, CHUNK_SIZE);
  int chunk_z = floorDiv(z, CHUNK_SIZE);
  int chunk_y = floorDiv(y, CHUNK_SIZE);

  ChunkCoord coord{chunk_x, chunk_z};
  ChunkColumn& column = chunkColumns[coord];

  if (!column.chunks[chunk_y]) {
    auto new_chunk = makeChunk(chunk_x, chunk_y, chunk_z);
    column.chunks[chunk_y] = new_chunk;
    pendingChunks.emplace_back(new_chunk);
  }

  return column.chunks[chunk_y];
}



Chunk* World::tryGetChunkPtrAt(int x, int y, int z) {
  int chunk_y = floorDiv(y, CHUNK_SIZE);
  if (chunk_y < 0 || chunk_y >= WORLD_HEIGHT_CHUNKS) {
    return nullptr;
  }

  int chunk_x = floorDiv(x, CHUNK_SIZE);
  int chunk_z = floorDiv(z, CHUNK_SIZE);

  ChunkCoord coord{chunk_x, chunk_z};
  auto column_it = chunkColumns.find(coord);
  if (column_it == chunkColumns.end()) {
    return nullptr;
  }

  ChunkColumn& column = column_it->second;
  const auto& chunk_ptr = column.chunks[chunk_y];
  if (!chunk_ptr) {
    return nullptr;
  }

  return chunk_ptr;
}


Block& World::getBlockAt(int x, int y, int z) {
  if (y < 0 || y >= WORLD_HEIGHT_CHUNKS * CHUNK_SIZE) {
    static Block air(BlockID::Air, {0,0,0});
    return air;
  }

  // Chunk* chunk = tryGetChunkAt(x, y, z); // return nullptr if missing
  Chunk& chunk = getChunkAt(x, y, z);

  int local_x = floorMod(x, CHUNK_SIZE);
  int local_y = floorMod(y, CHUNK_SIZE);
  int local_z = floorMod(z, CHUNK_SIZE);

  return chunk.blocks[local_x][local_y][local_z];
}


Block* World::tryGetBlockAt(int x, int y, int z) {
  if (y < 0 || y >= WORLD_HEIGHT_CHUNKS * CHUNK_SIZE) {
    return nullptr;
  }

  Chunk* chunk = tryGetChunkPtrAt(x, y, z);
  if (!chunk) {
    return nullptr;
  }

  int local_x = floorMod(x, CHUNK_SIZE);
  int local_y = floorMod(y, CHUNK_SIZE);
  int local_z = floorMod(z, CHUNK_SIZE);

  return &chunk->blocks[local_x][local_y][local_z];
}

void World::setBlockAtAndUpdate(int x, int y, int z, Block block) {
  Chunk& chunk = getChunkAt(x, y, z);

  int local_x = floorMod(x, CHUNK_SIZE);
  int local_y = floorMod(y, CHUNK_SIZE);
  int local_z = floorMod(z, CHUNK_SIZE);

  chunk.blocks[local_x][local_y][local_z] = block;
  chunk.isDirty = true;

  bool x_edge_s = local_x == 0;
  bool y_edge_s = local_y == 0;
  bool z_edge_s = local_z == 0;

  bool x_edge_e = local_x == CHUNK_SIZE - 1;
  bool y_edge_e = local_y == CHUNK_SIZE - 1;
  bool z_edge_e = local_z == CHUNK_SIZE - 1;

  if (x_edge_s || y_edge_s || z_edge_s ||
      x_edge_e || y_edge_e || z_edge_e) {

    if (x_edge_s) setChunkDirty(x - 1, y, z);
    if (y_edge_s) setChunkDirty(x, y - 1, z);
    if (z_edge_s) setChunkDirty(x, y, z - 1);

    if (x_edge_e) setChunkDirty(x + 1, y, z);
    if (y_edge_e) setChunkDirty(x, y + 1, z);
    if (z_edge_e) setChunkDirty(x, y, z + 1);
  }
}


void World::setBlockAt(int x, int y, int z, Block block) {
  Chunk& chunk = getChunkAt(x, y, z);

  int local_x = floorMod(x, CHUNK_SIZE);
  int local_y = floorMod(y, CHUNK_SIZE);
  int local_z = floorMod(z, CHUNK_SIZE);

  chunk.blocks[local_x][local_y][local_z] = block;
  chunk.isDirty = true;
}

void World::setChunkDirty(int x, int y, int z) {
  if (y < 0) return;
  getChunkAt(x, y, z).isDirty = true;
}


ChunkColumn& World::getOrCreateColumn(int cx, int cz) {

  ChunkCoord coord{cx, cz};
  auto it = chunkColumns.find(coord);
  if (it == chunkColumns.end()) {
    // create new column
    ChunkColumn column;
    chunkColumns[coord] = std::move(column);
    return chunkColumns[coord];
  }
  return it->second;
}

// void World::ensureChunkAndNeighbors(int worldX, int worldY, int worldZ) {
//   int cx = floorDiv(worldX, CHUNK_SIZE);
//   int cz = floorDiv(worldZ, CHUNK_SIZE);
//   int cy = floorDiv(worldY, CHUNK_SIZE);
//
//   // neighbor offsets
//   const std::array<std::tuple<int,int,int>, 7> offsets = {{
//     {0, 0, 0},   // self
//     { 1, 0, 0 },
//     {-1, 0, 0 },
//     { 0, 1, 0 },
//     { 0,-1, 0 },
//     { 0, 0, 1 },
//     { 0, 0,-1 }
//   }};
//
//   for (auto [dx, dy, dz] : offsets) {
//     int ncx = cx + dx;
//     int ncy = cy + dy;
//     int ncz = cz + dz;
//
//     if (ncy < 0 || ncy >= WORLD_HEIGHT_CHUNKS) continue;
//
//     ChunkColumn& col = getOrCreateColumn(ncx, ncz);
//
//     if (!col.chunks[ncy]) {
//       col.chunks[ncy] = std::make_unique<Chunk>(ncx, ncy, ncz);
//       // ^ generate terrain in constructor
//     }
//   }
// }

std::vector<Chunk*> World::ensureChunkAndNeighbors(
    int worldX, int worldY, int worldZ, int radius)
{
  std::vector<Chunk*> newChunks;
  newChunks.reserve((radius * 2 + 1) * (radius * 2 + 1) * WORLD_HEIGHT_CHUNKS);

  // Move pending chunks in once per call
  if (!pendingChunks.empty()) {
    newChunks.insert(
        newChunks.end(),
        std::make_move_iterator(pendingChunks.begin()),
        std::make_move_iterator(pendingChunks.end()));
    pendingChunks.clear();
  }

  const int cx = floorDiv(worldX, CHUNK_SIZE);
  const int cz = floorDiv(worldZ, CHUNK_SIZE);
  const float chunkHalf = CHUNK_SIZE * 0.5f;

  // --- Core loading loop ---
  // Process only the square area around the current player chunk.
  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dz = -radius; dz <= radius; ++dz) {

      const int ncx = cx + dx;
      const int ncz = cz + dz;

      // Try to find existing column
      auto it = chunkColumns.find({ncx, ncz});
      ChunkColumn* col = nullptr;

      if (it == chunkColumns.end()) {
        // Column doesn't exist → create it (only once)
        auto [newIt, inserted] = chunkColumns.emplace(
            ChunkCoord{ncx, ncz}, ChunkColumn{});
        col = &newIt->second;

        col->colCenter = {
          ncx * CHUNK_SIZE + chunkHalf,
          0.0f,
          ncz * CHUNK_SIZE + chunkHalf
      };
      } else {
        col = &it->second;
      }

      // Fill missing vertical chunks
      for (int ncy = 0; ncy < WORLD_HEIGHT_CHUNKS; ++ncy) {
        if (!col->chunks[ncy]) {
          Chunk* chunk = makeChunk(ncx, ncy, ncz);
          col->chunks[ncy] = chunk;
          newChunks.push_back(chunk);
        }
      }
    }
  }

  return newChunks;
}


std::optional<RaycastHit> World::raycastBlock(
  const glm::vec3 &origin,
  const glm::vec3 &direction,
  float maxDistance)
{
  glm::ivec3 voxel = glm::floor(origin);

  glm::ivec3 step(
      direction.x > 0 ? 1 : -1,
      direction.y > 0 ? 1 : -1,
      direction.z > 0 ? 1 : -1
  );

  glm::vec3 tMax;
  glm::vec3 tDelta;

  for (int i = 0; i < 3; i++) {
    if (direction[i] != 0) {
      float voxelBorder = (step[i] > 0 ? (voxel[i] + 1.0f) : voxel[i]);
      tMax[i] = (voxelBorder - origin[i]) / direction[i];
      tDelta[i] = step[i] / direction[i];
    } else {
      tMax[i] = FLT_MAX;
      tDelta[i] = FLT_MAX;
    }
  }

  float dist = 0.0f;
  glm::ivec3 hitNormal(0);

  while (dist <= maxDistance) {
    Block& block = getBlockAt(voxel.x, voxel.y, voxel.z);
    if (!block.isAir()) {
      return RaycastHit{voxel, hitNormal};
    }

    if (tMax.x < tMax.y) {
      if (tMax.x < tMax.z) {
        voxel.x += step.x;
        dist = tMax.x;
        tMax.x += tDelta.x;
        hitNormal = glm::ivec3(-step.x, 0, 0);
      } else {
        voxel.z += step.z;
        dist = tMax.z;
        tMax.z += tDelta.z;
        hitNormal = glm::ivec3(0, 0, -step.z);
      }
    } else {
      if (tMax.y < tMax.z) {
        voxel.y += step.y;
        dist = tMax.y;
        tMax.y += tDelta.y;
        hitNormal = glm::ivec3(0, -step.y, 0);
      } else {
        voxel.z += step.z;
        dist = tMax.z;
        tMax.z += tDelta.z;
        hitNormal = glm::ivec3(0, 0, -step.z);
      }
    }
  }

  return std::nullopt;
}


std::vector<std::reference_wrapper<Block>> World::getNearbyBlocks(glm::vec3 pos) {
  std::vector<std::reference_wrapper<Block>> result;

  glm::vec3 halfExtents = player_dimentions * 0.5f;
  glm::vec3 min = pos - halfExtents;
  glm::vec3 max = pos + halfExtents;

  // expand Y to full height
  min.y = pos.y;
  max.y = pos.y + player_dimentions.y;

  int minX = static_cast<int>(floor(min.x));
  int maxX = static_cast<int>(floor(max.x));
  int minY = static_cast<int>(floor(min.y));
  int maxY = static_cast<int>(floor(max.y));
  int minZ = static_cast<int>(floor(min.z));
  int maxZ = static_cast<int>(floor(max.z));

  for (int x = minX; x <= maxX; ++x) {
    for (int y = minY; y <= maxY; ++y) {
      for (int z = minZ; z <= maxZ; ++z) {
        Block& b = getBlockAt(x, y, z);
        if (!b.isAir()) {
          result.push_back(b);
        }
      }
    }
  }

  return result;
}


bool World::isAir(int x, int y, int z) {
  return isAir(glm::ivec3(x, y, z));
}

bool World::isAir(glm::ivec3 pos) {
  Block* block = tryGetBlockAt(pos.x, pos.y, pos.z);
  if (!block) {
    return true;
  }

  return block->isAir();
}


World::~World() {
  for (auto [coords, chunk] : chunkColumns) {
    for (auto ch : chunk.chunks) {
      if (ch) {
        ch->~Chunk();
        arena.deallocate((std::byte*)ch, 1);
      }
    }
  }
}

