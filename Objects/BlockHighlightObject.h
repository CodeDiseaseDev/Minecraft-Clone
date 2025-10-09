//
// Created by code on 9/25/25.
//

#ifndef BLOCKHIGHLIGHTOBJECT_H
#define BLOCKHIGHLIGHTOBJECT_H
#include <memory>

#include "../Shader.h"
#include "../Mesh.h"

class BlockHighlightObject {
public:

  Shader* shader;
  std::shared_ptr<Mesh> mesh;
  glm::vec3 chunk_pos {0,0,0};

  explicit BlockHighlightObject(Shader* s);

  void Draw(const glm::vec3 &blockPos, Camera &camera, const glm::vec4 &color);
};



#endif //BLOCKHIGHLIGHTOBJECT_H

