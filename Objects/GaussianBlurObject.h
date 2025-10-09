#ifndef GAUSSIANBLUROBJ_H
#define GAUSSIANBLUROBJ_H

#include <memory>
#include "../Shader.h"

class GaussianBlurObject {
public:
  GaussianBlurObject(Shader* shaderH,
    Shader* shaderV, int width, int height);

  // Apply blur on input texture, return blurred texture ID
  unsigned int Apply(unsigned int inputTex, int iterations = 10);

  // Draw fullscreen quad (public in case you need it elsewhere)
  void Draw();

private:
  unsigned int quadVAO, quadVBO;
  unsigned int pingpongFBO[2];
  unsigned int pingpongTex[2];
  int width, height;

  std::shared_ptr<Shader> blurShaderH;
  std::shared_ptr<Shader> blurShaderV;

  void initQuad();
  void initPingpong();

public:

  void updateSize(int newWidth, int newHeight);

};

#endif // GAUSSIANBLUROBJ_H
