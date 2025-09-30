//
// Created by code on 9/21/25.
//

#ifndef MESH_H
#define MESH_H



#include <vector>
#include <iostream>
#include <optional>

#include "Camera.h"
#include "Shader.h"
#include "tiny_obj_loader.h"
#include "Vertex.h"


#include "stb_image.h"

class Mesh {
public:
  GLuint vao = 0, vbo = 0, ebo = 0;
  GLsizei indexCount = 0;


  glm::vec3 position {0,0,0};
  glm::vec3 rotation {0,0,0};
  glm::vec3 scale    {1,1,1};

  glm::vec3 offset    {0,0,0};




  Mesh(const std::vector<Vertex>& vertices,
       const std::vector<unsigned int>& indices);

  // MOVE ONLY
  Mesh(Mesh&& other) noexcept;
  Mesh& operator=(Mesh&& other) noexcept;
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void useTransformation(glm::vec3 offset);

  void draw() const;
  glm::mat4 getModelMatrix() const;

  static std::optional<Mesh> loadOBJ(const std::string& path);

  void setPosition(const glm::vec3& p);
  void setRotation(const glm::vec3& r);
  void setScale(const glm::vec3& s);

  ~Mesh();
};



#endif //MESH_H

