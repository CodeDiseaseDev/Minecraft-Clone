#pragma once

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 texCoord;
  float aoFactor = 1.0f;
  glm::vec3 bary;
};