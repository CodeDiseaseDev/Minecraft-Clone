#ifndef SCENEFRAMEBUFFER_H
#define SCENEFRAMEBUFFER_H

#include <glad/gl.h>
#include <stdexcept>
#include <string>
#include <iostream>

class SceneFramebuffer {
public:
  SceneFramebuffer(int width, int height);
  ~SceneFramebuffer();

  void bind();        // bind for rendering
  void unbind();      // back to default framebuffer
  void resize(int newWidth, int newHeight); // realloc on resize

  unsigned int getTexture() const { return colorTex; }
  int getWidth()  const { return width; }
  int getHeight() const { return height; }

  void blitToDefault(int winW, int winH);

private:
  unsigned int FBO = 0;
  unsigned int colorTex = 0;
  unsigned int depthRBO = 0;
  int width, height;

  void init(int w, int h);
  void cleanup();
};

#endif // SCENEFRAMEBUFFER_H
