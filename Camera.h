//
// Created by code on 9/21/25.
//

#ifndef CAMERA_H
#define CAMERA_H


#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

#include "lighting_config.h"

// #include "Objects/GameObject.h"

struct Frustum {
  glm::vec4 planes[6]; // each plane is (a,b,c,d)
};

struct AABB;

class Camera {
public:
  glm::vec3 position;
  // glm::vec3 sunDirection{30, -90, 0};
  glm::vec3 rotation;
  glm::vec2 screenSize;

  lighting_config lighting_shader_config;



  float fov, aspect, nearPlane, farPlane;

  Camera(float aspectRatio);

  void setScreenSize(float width, float height);

  glm::mat4 getViewMatrix() const;

  glm::mat4 getProjectionMatrix() const;

  glm::vec3 getFront() const;
  glm::vec3 getRight() const;
  glm::vec3 getTop() const;

  bool isBoxInFrustum(AABB bounding_box);

  Frustum extractFrustum();

  glm::vec3 GetForwardVector() const;
};



#endif //CAMERA_H
