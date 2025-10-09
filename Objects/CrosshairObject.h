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
  CrosshairObject(Shader* shader, Texture* text);
  ~CrosshairObject();

  void Draw(int screenWidth, int screenHeight);

private:
  GLuint vao = 0;
  GLuint vbo = 0;
  Shader* shader;
  Texture* texture;
};



#endif //CROSSHAIROBJECT_H
