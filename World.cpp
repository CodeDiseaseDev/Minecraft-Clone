//
// Created by code on 9/23/25.
//

#include "World.h"

#include <cfloat>
#include <glm/common.hpp>
#include <iterator>
// #include <glm/detail/func_geometric.inl>

Chunk & World::getChunkAt(int x, int y, int z) {
  return *getChunkPtrAt(x, y, z);
}

std::shared_ptr<Chunk> World::makeChunk(int chunk_x, int chunk_y, int chunk_z) {
  return std::make_shared<Chunk>(chunk_x, chunk_y, chunk_z, seed);
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
    pendingChunks.emplace_back(std::move(new_chunk));
  }

  return column.chunks[chunk_y].get();
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

  return chunk_ptr.get();
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


void World::setBlockAt(int x, int y, int z, Block block) {
  Chunk& chunk = getChunkAt(x, y, z);

  int local_x = floorMod(x, CHUNK_SIZE);
  int local_y = floorMod(y, CHUNK_SIZE);
  int local_z = floorMod(z, CHUNK_SIZE);

  chunk.blocks[local_x][local_y][local_z] = block;

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

std::vector<std::shared_ptr<Chunk>> World::ensureChunkAndNeighbors(
  int worldX, int worldY, int worldZ, int radius) {

  std::vector<std::shared_ptr<Chunk>> new_chunks;

  if (!pendingChunks.empty()) {
    new_chunks.insert(
      new_chunks.end(),
      std::make_move_iterator(pendingChunks.begin()),
      std::make_move_iterator(pendingChunks.end()));
    pendingChunks.clear();
  }

  int cx = floorDiv(worldX, CHUNK_SIZE);
  int cz = floorDiv(worldZ, CHUNK_SIZE);

  for (auto [coords, chunk] : chunkColumns) {
    float distance = glm::distance(
      glm::vec3(coords.x,WORLD_HEIGHT_CHUNKS,coords.y),
      glm::vec3(worldX, worldY, worldZ));

    if (distance >= radius) {

      // chunkColumns.erase(coords);
    }
  }

  // bool should_update = false;
  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dz = -radius; dz <= radius; ++dz) {
      // if (dx*dx + dz*dz > radius*radius) continue;

      int ncx = cx + dx;
      int ncz = cz + dz;

      // ChunkColumn& col = getOrCreateColumn(ncx, ncz);

      glm::vec3 colCenter(
        ncx * CHUNK_SIZE + CHUNK_SIZE * 0.5f,
        0, // ignore height for distance
        ncz * CHUNK_SIZE + CHUNK_SIZE * 0.5f
      );

      auto [it, inserted] = chunkColumns.try_emplace({ncx, ncz});
      ChunkColumn& col = it->second;

      col.colCenter = colCenter;

      for (int ncy = 0; ncy < WORLD_HEIGHT_CHUNKS; ++ncy) {
        if (!col.chunks[ncy]) {
          auto chunk = makeChunk(ncx, ncy, ncz);
          col.chunks[ncy] = chunk;
          new_chunks.emplace_back(std::move(chunk));
        }
      }
    }
  }

  return new_chunks;
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

    // Step to next voxel
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


std::vector<std::reference_wrapper<Block>> World::getNearbyBlocks(glm::vec3 vec) {
  std::vector<std::reference_wrapper<Block>> result;

  // The player only collides with blocks in a small area around them.
  // Let’s say a 3×3×3 cube centered at player position.
  int px = static_cast<int>(floor(vec.x));
  int py = static_cast<int>(floor(vec.y));
  int pz = static_cast<int>(floor(vec.z));

  for (int x = px - 1; x <= px + 1; ++x) {
    for (int y = py - 1; y <= py + 2; ++y) {   // +2 so we check blocks above player head
      for (int z = pz - 1; z <= pz + 1; ++z) {
        Block& b = getBlockAt(x, y, z); // however you access blocks
        if (!b.isAir()) {
          result.push_back(b);
        }
      }
    }
  }

  return result;
}

bool World::isAir(int x, int y, int z) {
  Block* block = tryGetBlockAt(x, y, z);
  if (!block) {
    return true;
  }

  return block->isAir();
}

