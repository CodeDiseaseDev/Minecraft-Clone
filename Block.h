#pragma once
#include <glm/ext/scalar_uint_sized.hpp>

#include "Objects/GameObject.h"

enum class BlockID : glm::uint8 {
  Air = 0,
  Grass = 1,
  Stone = 2,
};

class Block {
public:
  BlockID id = BlockID::Air;
  glm::vec3 position = glm::vec3(0, 0, 0);

  AABB getAABB() {
    return AABB(
      position,
      glm::vec3(1,1,1));
  }
  // ...

  [[nodiscard]] bool isAir() const
    { return id == BlockID::Air; }
};
