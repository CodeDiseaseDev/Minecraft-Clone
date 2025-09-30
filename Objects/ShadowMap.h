//
// Created by code on 9/28/25.
//

#ifndef SHADOWMAP_H
#define SHADOWMAP_H
#include <memory>

#include "GameObject.h"
#include "../Shader.h"


#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ShadowMap: public GameObject {
public:
  ShadowMap(std::shared_ptr<Shader> &depthShader, int resolution = 2048);
  ~ShadowMap();

  void Resize(int resolution);
  void BeginDepthPass(glm::mat4 lightSpaceMatrix);
  void EndDepthPass();

  GLuint GetDepthMap() const { return depthMap; }
  glm::mat4 GetLightSpaceMatrix(const glm::vec3& sunDir,
                                         const glm::vec3& center);

  const glm::mat4& LastLightSpace() const { return lastLightSpace_; }
  const glm::vec3& LastLightPosition() const { return lastLightPos_; }

  // Camera GetLightCamera(const glm::vec3& sunDir) const;

  void bind(int unit);

private:
  GLuint depthMapFBO = 0;
  GLuint depthMap = 0;
  int resolution;

  glm::mat4 lastLightSpace_{1.0f};
  glm::vec3 lastLightPos_{0.0f};

  std::shared_ptr<Shader>& shader;

  void Init(int res);


};



#endif //SHADOWMAP_H
