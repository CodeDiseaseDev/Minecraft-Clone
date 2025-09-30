//
// Created by code on 9/22/25.
//

#include "MeshObject.h"

MeshObject::MeshObject(
  std::unique_ptr<Mesh>&& m,
  std::shared_ptr<Shader>& shader):
    mesh(std::move(m)), shader(shader) {}

void MeshObject::Update(float dt) {
  // e.g., auto-rotate
  // rotation.y += 20.0f * dt;
}

void MeshObject::Draw(Camera &camera) const {
  shader->use();
  shader->useCamera(camera);
  shader->setMat4("model", getModelMatrix());
  shader->setVec4("_color", glm::vec4(1,1,0,1));

  mesh->draw();
}

