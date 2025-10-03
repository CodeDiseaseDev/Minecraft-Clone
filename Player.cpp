//
// Created by code on 9/26/25.
//

#include "Player.h"

#include <cstdio>
#include <stdexcept>

Player::Player() {

  bounding_box = {
    glm::vec3(-player_dimentions.x / 2,  0,                  -player_dimentions.z / 2),
    glm::vec3( player_dimentions.x / 2,  player_dimentions.y, player_dimentions.z / 2)
  };
}

AABB Player::getAABB() {
  return {
    bounding_box.min + position,
    bounding_box.max + position
  };
}

void Player::useCamera(Camera& camera, bool thirdPerson) {
  // Player head position
  glm::vec3 eyePos(
      position.x,
      position.y + player_dimentions.y,
      position.z
  );

  if (!thirdPerson) {
    // First person
    camera.position = eyePos;
    camera.rotation = rotation;
  } else {
    throw std::runtime_error("Not implemented");
  }
}




bool Player::isStandingOnBlock(World &world) {
  float footY = position.y;
  float epsilon = 0.05f;

  // Player's footprint (width/2)
  float halfWidth = 0.3f; // ~0.6 wide like MC
  float halfDepth = 0.3f;

  // Candidate positions under each corner
  std::vector<glm::vec3> checkPoints = {
    { position.x - halfWidth, footY - epsilon, position.z - halfDepth },
    { position.x + halfWidth, footY - epsilon, position.z - halfDepth },
    { position.x - halfWidth, footY - epsilon, position.z + halfDepth },
    { position.x + halfWidth, footY - epsilon, position.z + halfDepth },
};

  for (auto& p : checkPoints) {
    int blockX = static_cast<int>(std::floor(p.x));
    int blockY = static_cast<int>(std::floor(p.y));
    int blockZ = static_cast<int>(std::floor(p.z));

    auto block = world.getBlockAt(blockX, blockY, blockZ);
    if (!block.isAir()) {
      return true; // standing on something
    }
  }

  return false;
}


void Player::gravityTick(World& world, float dt) {
  if (!is_on_ground) {
    gravity_velocity.y += world.gravity * dt;  // accelerate downward
  }

  // If standing on block, prevent accumulating downward velocity
  if (isStandingOnBlock(world) && gravity_velocity.y < 0.0f) {
    gravity_velocity.y = 0.0f;
    is_on_ground = true;
  } else {
    is_on_ground = false;
  }
}


Player::~Player() {
}
