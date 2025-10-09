//
// Created by code on 9/22/25.
//

#include "MeshObject.h"

MeshObject::MeshObject(
  std::unique_ptr<Mesh>& m,
  std::shared_ptr<Shader>& shader):
    mesh(std::move(m)), shader(shader) {}

void MeshObject::Update(float dt) {
  // e.g., auto-rotate
  // rotation.y += 20.0f * dt;
}

void MeshObject::draw(Camera &camera, std::shared_ptr<ShadowMap> shadow_map) const {
  shader->use();
  shader->useCamera(camera);
  shader->useCameraWorldMesh(camera);
  shader->useCameraLighting(camera);

  shader->setMat4("model", getModelMatrix());
  shader->setVec4("_color", glm::vec4(1,1,0,1));

  // texture_atlas->bind(0);
  shader->setInt("useAtlas", 0);

  if (shadow_map) {
    shader->setMat4("lightSpaceMatrix", shadow_map->LastLightSpace());
    shader->setVec3("lightPos", shadow_map->LastLightPosition());
    shadow_map->bind(1);
  } else {
    shader->setMat4("lightSpaceMatrix", glm::mat4(1.0f));
    shader->setVec3("lightPos", glm::vec3(0.0f));
  }
  shader->setInt("shadowMap", 1);

  mesh->draw();
}

