//
// Created by code on 9/22/25.
//

#include "GameObject.h"

void GameObject::Update(float dt) {}

void GameObject::Draw(Camera& camera) const {}

glm::mat4 GameObject::getModelMatrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, position);
  model = glm::rotate(model, glm::radians(rotation.x), {1,0,0});
  model = glm::rotate(model, glm::radians(rotation.y), {0,1,0});
  model = glm::rotate(model, glm::radians(rotation.z), {0,0,1});
  model = glm::scale(model, scale);
  return model;
}

AABB GameObject::getBoundingBox() const {
  return AABB{
    position,
    scale + position
  };
}
