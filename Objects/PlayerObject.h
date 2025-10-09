//
// Created by code on 10/1/25.
//

#ifndef PLAYEROBJECT_H
#define PLAYEROBJECT_H


#include "../Mesh.h"
#include "../Shader.h"
#include "../Texture.h"
#include "../Camera.h"
#include "ShadowMap.h"

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class PlayerObject {
public:
  PlayerObject(arena::Allocator<std::byte>& arena, Shader* s, Texture* tex);

  arena::Allocator<std::byte>& arena;

  void update(float dt);
  void draw(Camera& camera, ShadowMap* shadow_map) const;

  // Transform
  glm::vec3 position {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation {0.0f, 0.0f, 0.0f}; // yaw/pitch/roll for whole body

  // Animation angles
  float leftArmAngle  = 0.0f;
  float rightArmAngle = 0.0f;
  float leftLegAngle  = 0.0f;
  float rightLegAngle = 0.0f;

private:
  Shader* shader;
  Texture* texture_atlas;

  // Meshes for body parts
  Mesh* torso;
  Mesh* head;
  Mesh* leftArm;
  Mesh* rightArm;
  Mesh* leftLeg;
  Mesh* rightLeg;

  Mesh* makeCubeMesh(glm::vec3 size);


};



#endif //PLAYEROBJECT_H
