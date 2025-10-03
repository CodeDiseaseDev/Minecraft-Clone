//
// Created by code on 9/25/25.
//

#include "BlockHighlightObject.h"


BlockHighlightObject::BlockHighlightObject(std::shared_ptr<Shader> &s)
    : shader(s)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  float val = 0.505f; // half-extent (slightly >0.5 to cover the block)

  glm::vec3 positions[8] = {
    {-val, -val, -val},
    { val, -val, -val},
    { val,  val, -val},
    {-val,  val, -val},
    {-val, -val,  val},
    { val, -val,  val},
    { val,  val,  val},
    {-val,  val,  val}
  };

  auto addFace = [&](int i0, int i1, int i2, int i3, glm::vec3 normal) {
    unsigned int start = vertices.size();

    vertices.push_back({positions[i0], normal});
    vertices.push_back({positions[i1], normal});
    vertices.push_back({positions[i2], normal});
    vertices.push_back({positions[i3], normal});

    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 0);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
  };

  // Front (+Z)
  addFace(4, 5, 6, 7, {0, 0, 1});
  // Back (-Z)
  addFace(1, 0, 3, 2, {0, 0, -1});
  // Left (-X)
  addFace(0, 4, 7, 3, {-1, 0, 0});
  // Right (+X)
  addFace(5, 1, 2, 6, {1, 0, 0});
  // Bottom (-Y)
  addFace(0, 1, 5, 4, {0, -1, 0});
  // Top (+Y)
  addFace(3, 7, 6, 2, {0, 1, 0});

  mesh = std::make_unique<Mesh>(vertices, indices);
}

void BlockHighlightObject::Draw(const glm::vec3 &blockPos, Camera &camera, const glm::vec4 &color) {
  shader->use();
  shader->useCamera(camera);

  glm::mat4 model = glm::translate(glm::mat4(1.0f), blockPos);
  shader->setMat4("model", model);
  shader->setVec4("highlightColor", color);

  // Optionally wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  mesh->draw();
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
