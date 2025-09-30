//
// Created by code on 9/25/25.
//

#include "CrosshairObject.h"

CrosshairObject::CrosshairObject(std::shared_ptr<Shader>& shader, std::shared_ptr<Texture>& text)
    : shader(shader), texture(text)
{
  float vertices[] = {
    // pos      // tex
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 1.0f
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);
}

CrosshairObject::~CrosshairObject() {
  if (vbo) glDeleteBuffers(1, &vbo);
  if (vao) glDeleteVertexArrays(1, &vao);
}

void CrosshairObject::Draw(int screenWidth, int screenHeight) {
  glDisable(GL_DEPTH_TEST); // always on top

  // printf("%i, %i\n", screenWidth, screenHeight);

  shader->use();
  shader->setVec2("screenSize", {screenWidth, screenHeight});
  shader->setVec2("crosshairSize", glm::vec2(texture->width, texture->height));

  texture->bind(0);
  shader->setInt("crossTex", 0);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // textured quad
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);
}
