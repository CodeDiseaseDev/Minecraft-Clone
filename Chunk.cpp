//
// Created by code on 9/23/25.
//

#include "Chunk.h"

#include <cmath>
#include <memory>
#include <cstdlib>

#include "FastNoiseLite.h"

Chunk::Chunk(int chunkX, int chunkY, int chunkZ, int seed) {
  // position = glm::vec3(chunkX, chunkY, chunkZ);
  // for (int x = 0; x < CHUNK_SIZE; x++) {
  //   for (int z = 0; z < CHUNK_SIZE; z++) {
  //     // world-space coords
  //     int worldX = chunkX * CHUNK_SIZE + x;
  //     int worldZ = chunkZ * CHUNK_SIZE + z;
  //
  //     // simple height function (sin waves, random noise, etc.)
  //     int groundHeight = 8 + (std::sin(worldX * 0.1) + std::cos(worldZ * 0.1)) * 3;
  //
  //     for (int y = 0; y < CHUNK_SIZE; y++) {
  //       int worldY = chunkY * CHUNK_SIZE + y;
  //
  //       if (worldY < groundHeight) {
  //         blocks[x][y][z] = Block{ BlockID::Air };
  //       } else if (worldY == groundHeight) {
  //         blocks[x][y][z] = Block{ BlockID::Grass };
  //       } else {
  //         blocks[x][y][z] = Block{ BlockID::Air };
  //       }
  //
  //       blocks[x][y][z].position = glm::vec3(x, y, z);
  //     }
  //   }
  // }

  position = glm::vec3(chunkX, chunkY, chunkZ);

  // Setup noise generator
  FastNoiseLite noise;
  noise.SetSeed(seed);
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFrequency(0.01f); // controls terrain "stretch"

  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      // World coords
      int worldX = chunkX * CHUNK_SIZE + x;
      int worldZ = chunkZ * CHUNK_SIZE + z;

      // Sample noise
      float n = noise.GetNoise((float)worldX, (float)worldZ);

      // Map noise [-1,1] → usable terrain height
      int groundHeight = 20 + (int)(n * 10.0f);

      for (int y = 0; y < CHUNK_SIZE; y++) {
        int worldY = chunkY * CHUNK_SIZE + y;

        if (worldY < groundHeight - 3) {
          blocks[x][y][z] = Block{ BlockID::Air };
        }
        else if (worldY < groundHeight) {
          blocks[x][y][z] = Block{ BlockID::Grass };
        }
        else {
          blocks[x][y][z] = Block{ BlockID::Air };
        }

        blocks[x][y][z].position = glm::vec3(x, y, z);
      }
    }
  }
}

