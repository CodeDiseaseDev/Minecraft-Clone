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
  PlayerObject(std::shared_ptr<Shader> s, std::shared_ptr<Texture> tex);

  void update(float dt);
  void draw(Camera& camera, std::shared_ptr<ShadowMap> shadow_map) const;

  // Transform
  glm::vec3 position {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation {0.0f, 0.0f, 0.0f}; // yaw/pitch/roll for whole body

  // Animation angles
  float leftArmAngle  = 0.0f;
  float rightArmAngle = 0.0f;
  float leftLegAngle  = 0.0f;
  float rightLegAngle = 0.0f;

private:
  std::shared_ptr<Shader> shader;
  std::shared_ptr<Texture> texture_atlas;

  // Meshes for body parts
  std::unique_ptr<Mesh> torso;
  std::unique_ptr<Mesh> head;
  std::unique_ptr<Mesh> leftArm;
  std::unique_ptr<Mesh> rightArm;
  std::unique_ptr<Mesh> leftLeg;
  std::unique_ptr<Mesh> rightLeg;

  std::unique_ptr<Mesh> makeCubeMesh(glm::vec3 size);
};



#endif //PLAYEROBJECT_H
