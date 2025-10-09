//
// Created by code on 9/23/25.
//

#include "Chunk.h"

#include <cmath>
#include <memory>
#include <cstdlib>
#include <vector>

#include "FastNoiseLite.h"

glm::ivec3 Chunk::toWorldCoords(int lx, int ly, int lz) const {
  return {
    position.x * CHUNK_SIZE + lx,
    position.y * CHUNK_SIZE + ly,
    position.z * CHUNK_SIZE + lz
  };
}


Chunk::Chunk(int chunkX, int chunkY, int chunkZ, int seed) {

  position = glm::ivec3(chunkX, chunkY, chunkZ);

  // Setup noise generator
  FastNoiseLite noise;
  noise.SetSeed(seed);
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFrequency(0.01f); // controls terrain "stretch"

  FastNoiseLite ore_noise;
  ore_noise.SetSeed(seed + 1234);
  ore_noise.SetFrequency(0.05f);
  ore_noise.SetDomainWarpAmp(20.0f);
  ore_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  FastNoiseLite ground_level_stone;
  ground_level_stone.SetSeed(seed + 4321);
  ground_level_stone.SetFrequency(0.1f);
  ground_level_stone.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      // World coords
      int worldX = chunkX * CHUNK_SIZE + x;
      int worldZ = chunkZ * CHUNK_SIZE + z;



      // Sample noise
      float n = noise.GetNoise((float)worldX, (float)worldZ);

      // Map noise [-1,1] → usable terrain height
      int groundHeight = 20 + (int)(n * 20.0f);

      for (int y = 0; y < CHUNK_SIZE; y++) {
        int worldY = chunkY * CHUNK_SIZE + y;

        float ore_noise_val = ore_noise.GetNoise(
          (float)worldX, (float)worldY, (float)worldZ);

        float ground_level_val = ground_level_stone.GetNoise(
          (float)worldX, (float)worldZ);

        if (worldY < groundHeight) {
          if (worldY < groundHeight - 3) {
            if (ore_noise_val >= 0.95f) {
              blocks[x][y][z] = Block{ BlockID::DiamondOre };
            }
            else {
              blocks[x][y][z] = Block{ BlockID::Stone };
            }
          }
          else if (ground_level_val >= 0.95f) {
            blocks[x][y][z] = Block{ BlockID::Stone };
          }
          else if (worldY < groundHeight - 1) {
            blocks[x][y][z] = Block{ BlockID::Dirt };
          }
          else if (worldY < groundHeight) {
            blocks[x][y][z] = Block{ BlockID::Grass };
          }


          if (worldY > 10) {
            blocks[x][y][z] = Block{ BlockID::Air };

          }
        }
        else {
          blocks[x][y][z] = Block{ BlockID::Air };
        }

        blocks[x][y][z].position = glm::ivec3(
          chunkX * CHUNK_SIZE + x,
          chunkY * CHUNK_SIZE + y,
          chunkZ * CHUNK_SIZE + z);
      }
    }
  }
}

