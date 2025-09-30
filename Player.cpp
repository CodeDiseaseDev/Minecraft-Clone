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
  // Bottom of the player's bounding box
  float footY = position.y;

  // Small epsilon so floating-point doesn't fail
  float epsilon = 0.05f;

  // Check at foot level just below player
  int blockX = static_cast<int>(std::floor(position.x));
  int blockY = static_cast<int>(std::floor(footY - epsilon));
  int blockZ = static_cast<int>(std::floor(position.z));

  auto block = world.getBlockAt(blockX, blockY, blockZ);
  return !block.isAir(); // true if it's a solid block
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
