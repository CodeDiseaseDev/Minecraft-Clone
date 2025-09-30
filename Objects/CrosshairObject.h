//
// Created by code on 9/25/25.
//

#ifndef CROSSHAIROBJECT_H
#define CROSSHAIROBJECT_H
#include <memory>

#include "../Shader.h"
#include "../Texture.h"


class CrosshairObject {
public:
  CrosshairObject(std::shared_ptr<Shader>& shader, std::shared_ptr<Texture>& text);
  ~CrosshairObject();

  void Draw(int screenWidth, int screenHeight);

private:
  GLuint vao = 0;
  GLuint vbo = 0;
  std::shared_ptr<Shader>& shader;
  std::shared_ptr<Texture>& texture;
};



#endif //CROSSHAIROBJECT_H
