//
// Created by code on 9/25/25.
//

#ifndef BLOCKHIGHLIGHTOBJECT_H
#define BLOCKHIGHLIGHTOBJECT_H
#include <memory>

#include "../Shader.h"
#include "../Mesh.h"
#include "../World.h"

class BlockHighlightObject {
public:

  Shader* shader;
  Mesh* mesh;
  glm::vec3 chunk_pos {0,0,0};

  explicit BlockHighlightObject(arena::Allocator<std::byte>& arena, Shader* s);

  void Draw(const RaycastHit &hit, Camera &camera, const glm::vec4 &color);
};



#endif //BLOCKHIGHLIGHTOBJECT_H

