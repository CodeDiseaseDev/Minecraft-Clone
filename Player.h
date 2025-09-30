//
// Created by code on 9/26/25.
//

#ifndef PLAYER_H
#define PLAYER_H
#include "World.h"
#include "Objects/GameObject.h"

glm::vec3 const player_dimentions = glm::vec3(
  0.6, 1.8, 0.6
);

class Player {
public:
  AABB bounding_box;

  glm::vec3 position {0, 100, 0};
  glm::vec3 gravity_velocity;
  glm::vec3 rotation;

  bool is_on_ground;

  Player();

  AABB getAABB();

  void useCamera(Camera& camera, bool thirdPerson = false);
  bool isStandingOnBlock(World& world);
  void gravityTick(World& world, float deltaTime);

  ~Player();
};



#endif //PLAYER_H
