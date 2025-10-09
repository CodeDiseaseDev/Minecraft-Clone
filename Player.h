//
// Created by code on 9/26/25.
//

#ifndef PLAYER_H
#define PLAYER_H
#include "EaseVec3.h"
#include "World.h"
#include "Objects/GameObject.h"

glm::vec3 const player_dimentions = glm::vec3(
  0.6, 1.7, 0.6
);

class Player {
public:
  AABB bounding_box;

  glm::vec3 position {0, 100, 0};
  glm::vec3 gravity_velocity;
  glm::vec3 jump_velocity;

  glm::vec3 rotation;

  float jumpStrength = 9.0f;

  bool is_on_ground;

  bool is_sprinting = false;


  Player();

  AABB getAABB();

  void useCamera(Camera& camera, bool thirdPerson = false);
  glm::vec3 tryMoveWithSlide(World* world, glm::vec3 desiredPos, glm::vec3 velocity, float dt);
  bool canMoveTo(World* world, glm::vec3 newPos);
  bool isStandingOnBlock(World* world);
  void jumpTick(World* world, bool spaceHeld, float dt);
  void gravityTick(World* world, float deltaTime);

  ~Player();
};



#endif //PLAYER_H
