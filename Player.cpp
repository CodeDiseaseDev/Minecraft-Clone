//
// Created by code on 9/26/25.
//

#include "Player.h"

#include <cstdio>
#include <stdexcept>

#include "EaseValue.h"

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

void Player::useCamera(Camera& camera, bool updatePos, bool thirdPerson) {
  if (thirdPerson) {
    throw std::runtime_error("lol nah");
  }

  if (updatePos) {
    camera.position = get_player_eye_pos();
    camera.rotation = rotation;
  }
}

glm::vec3 Player::tryMoveWithSlide(World* world, glm::vec3 desiredPos, glm::vec3 velocity, float dt) {
  glm::vec3 finalPos = position;
  glm::vec3 move = velocity * dt;

  // --- Try X movement first ---
  glm::vec3 testPosX = finalPos + glm::vec3(move.x, 0.0f, 0.0f);
  if (canMoveTo(world, testPosX)) {
    finalPos.x = testPosX.x;
  } else {
    move.x = 0.0f; // Blocked on X axis
  }

  // --- Try Z movement next ---
  glm::vec3 testPosZ = finalPos + glm::vec3(0.0f, 0.0f, move.z);
  if (canMoveTo(world, testPosZ)) {
    finalPos.z = testPosZ.z;
  } else {
    move.z = 0.0f; // Blocked on Z axis
  }

  // --- Y movement (gravity / jumping) ---
  glm::vec3 testPosY = finalPos + glm::vec3(0.0f, move.y, 0.0f);
  if (canMoveTo(world, testPosY)) {
    finalPos.y = testPosY.y;
  } else {
    move.y = 0.0f; // Hit floor or ceiling
  }

  return finalPos;


  // glm::vec3 move = velocity * dt;
  // glm::vec3 allowed = move;
  //
  // // --- X axis ---
  // glm::vec3 testPosX = position + glm::vec3(move.x, 0.0f, 0.0f);
  // if (!canMoveTo(world, testPosX))
  //   allowed.x = 0.0f;
  //
  // // --- Z axis ---
  // glm::vec3 testPosZ = position + glm::vec3(0.0f, 0.0f, move.z);
  // if (!canMoveTo(world, testPosZ))
  //   allowed.z = 0.0f;
  //
  // // --- Y axis ---
  // glm::vec3 testPosY = position + glm::vec3(0.0f, move.y, 0.0f);
  // if (!canMoveTo(world, testPosY))
  //   allowed.y = 0.0f;
  //
  // // Return velocity actually permitted by collisions
  // return allowed / dt;
}


bool Player::canMoveTo(World* world, glm::vec3 newPos) {
  // Player bounding box (roughly 0.6 × 1.8 × 0.6)
  const float halfWidth = 0.3f;
  const float halfDepth = 0.3f;
  const float height = 1.8f;

  // Step height tolerance — how high we can step before collision stops us
  const float stepHeight = 0.2f;

  // Sample points around the player’s sides (bottom + mid + top)
  std::vector<glm::vec3> checkPoints;

  for (float y = 0.0f; y <= height; y += height / 2.0f) {
    checkPoints.push_back({ newPos.x - halfWidth, newPos.y + y, newPos.z - halfDepth });
    checkPoints.push_back({ newPos.x + halfWidth, newPos.y + y, newPos.z - halfDepth });
    checkPoints.push_back({ newPos.x - halfWidth, newPos.y + y, newPos.z + halfDepth });
    checkPoints.push_back({ newPos.x + halfWidth, newPos.y + y, newPos.z + halfDepth });
  }

  // Check if any corner collides with a solid block
  for (auto& p : checkPoints) {
    int bx = static_cast<int>(std::floor(p.x));
    int by = static_cast<int>(std::floor(p.y));
    int bz = static_cast<int>(std::floor(p.z));

    const Block& block = world->getBlockAt(bx, by, bz);
    if (!block.isAir()) {
      // Allow small step-ups (like stairs)
      const Block& above = world->getBlockAt(bx, by + 1, bz);
      if (above.isAir()) {
        // Check if block is short enough to step onto
        float blockTop = by + 1.0f;
        if (newPos.y + stepHeight >= blockTop)
          continue;
      }
      return false;
    }
  }

  return true;
}


bool Player::isStandingOnBlock(World* world) {
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

    auto block = world->getBlockAt(blockX, blockY, blockZ);
    if (!block.isAir()) {
      return true; // standing on something
    }
  }

  return false;
}

void Player::jumpTick(World* world, bool spaceHeld, float dt) {
  static bool jumpPressedLastFrame = false;
  bool jumpJustPressed = (spaceHeld && !jumpPressedLastFrame);

  static EaseValue vel_y;

  if (spaceHeld && is_on_ground) {
    float sprint_multiplier = is_sprinting ? 1.3f : 1.0f;


    // velocity will SNAP directly to max
    vel_y.value = jumpStrength * sprint_multiplier;
    vel_y.target = vel_y.value;


    // is_on_ground = false;
  }
  else if (!jumpJustPressed) {
    // the velocity will ease down
    vel_y.target = 0.0f;
  }



  vel_y.tick(dt, 3.0f);

  jump_velocity = glm::vec3(0.0f, vel_y.value, 0.0f);


  jumpPressedLastFrame = spaceHeld;
}











void Player::gravityTick(World* world, float dt) {
  // Apply gravity if airborne
  gravity_velocity.x = 0;
  gravity_velocity.z = 0;

  static EaseValue vel_y;



  vel_y.tick(dt, 80.0f);

  // bool landed = false;

  // Check if landed
  if (isStandingOnBlock(world)) {
    if (gravity_velocity.y < 0.0f)
      vel_y.target = vel_y.value = gravity_velocity.y = 0.0f;

    // position.y = floor(position.y);
    // landed = true;
    is_on_ground = true;
  } else {
    is_on_ground = false;
  }

  if (!is_on_ground) {
    vel_y.target += world->gravity * dt; // accumulate downward accel
    gravity_velocity.y = vel_y.value;
  }
}




Player::~Player() {
}

glm::vec3 Player::getFront() const {
  float pitch = glm::radians(rotation.y); // rotation around X axis
  float yaw   = glm::radians(rotation.x); // rotation around Y axis

  glm::vec3 front;
  front.x = cos(pitch) * sin(yaw);
  front.y = sin(pitch);               // yes, this controls Y movement (up/down)
  front.z = cos(pitch) * cos(yaw);
  return glm::normalize(front);

}

glm::vec3 Player::getRight() const {
  return glm::normalize(glm::cross(getFront(), glm::vec3(0,1,0)));
}
