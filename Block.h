#pragma once
#include <unordered_map>
#include <glm/ext/scalar_uint_sized.hpp>
#include <glm/vec3.hpp>

#include "Objects/GameObject.h"

enum class BlockID : glm::uint8 {
  Air = 0,
  Grass = 1,
  Stone = 2,
  Dirt = 3,
  DiamondOre = 4,
};

struct BlockUV {
  glm::vec2 faces[6]; // +X, -X, +Y, -Y, +Z, -Z
};

class Block {
public:
  BlockID id = BlockID::Air;
  glm::ivec3 position {0, 0, 0};

  AABB getAABB() const {
    glm::vec3 min = glm::vec3(position);
    return AABB(
      min,
      min + glm::vec3(1.0f));
  }
  // ...

  bool isAir() const
    { return id == BlockID::Air; }
};
