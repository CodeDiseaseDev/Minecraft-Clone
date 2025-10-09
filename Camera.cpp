//
// Created by code on 9/21/25.
//

#include "Camera.h"

#include "World.h"
#include "Objects/GameObject.h"

Camera::Camera(float aspectRatio):
                              fov(45.0f), aspect(aspectRatio),
                              nearPlane(0.02f), farPlane(1000.0f),
                              rotation(0,0,0) {}

void Camera::setScreenSize(float width, float height) {
  screenSize = glm::vec2(width, height);
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + getFront(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix() const {
  return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

glm::vec3 Camera::getFront() const {
  glm::vec3 front;
  front.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
  front.y = sin(glm::radians(rotation.y));
  front.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
  return glm::normalize(front);
}

glm::vec3 Camera::getRight() const {
  return glm::normalize(glm::cross(getFront(), glm::vec3(0,1,0)));
}



Frustum Camera::extractFrustum() {
  Frustum f;

  glm::mat4 VP = getProjectionMatrix() * getViewMatrix();

  // Left
  f.planes[0] = glm::vec4(
      VP[0][3] + VP[0][0],
      VP[1][3] + VP[1][0],
      VP[2][3] + VP[2][0],
      VP[3][3] + VP[3][0]);

  // Right
  f.planes[1] = glm::vec4(
      VP[0][3] - VP[0][0],
      VP[1][3] - VP[1][0],
      VP[2][3] - VP[2][0],
      VP[3][3] - VP[3][0]);

  // Bottom
  f.planes[2] = glm::vec4(
      VP[0][3] + VP[0][1],
      VP[1][3] + VP[1][1],
      VP[2][3] + VP[2][1],
      VP[3][3] + VP[3][1]);

  // Top
  f.planes[3] = glm::vec4(
      VP[0][3] - VP[0][1],
      VP[1][3] - VP[1][1],
      VP[2][3] - VP[2][1],
      VP[3][3] - VP[3][1]);

  // Near
  f.planes[4] = glm::vec4(
      VP[0][3] + VP[0][2],
      VP[1][3] + VP[1][2],
      VP[2][3] + VP[2][2],
      VP[3][3] + VP[3][2]);

  // Far
  f.planes[5] = glm::vec4(
      VP[0][3] - VP[0][2],
      VP[1][3] - VP[1][2],
      VP[2][3] - VP[2][2],
      VP[3][3] - VP[3][2]);

  // Normalize each plane
  for (int i = 0; i < 6; i++) {
    float length = glm::length(glm::vec3(f.planes[i]));
    f.planes[i] /= length;
  }

  return f;
}

glm::vec3 Camera::GetForwardVector() const {
  float yawRad = glm::radians(rotation.y);
  float pitchRad = glm::radians(rotation.x);

  return glm::normalize(glm::vec3(
      cos(pitchRad) * cos(yawRad),
      sin(pitchRad),
      cos(pitchRad) * sin(yawRad)
  ));
}

bool Camera::isBoxInFrustum(AABB bounding_box) {
  Frustum f = extractFrustum();
  for (int i = 0; i < 6; i++) {
    const glm::vec4& p = f.planes[i];

    // Compute the vertex farthest in plane normal direction
    glm::vec3 positive = bounding_box.min;
    if (p.x >= 0) positive.x = bounding_box.max.x;
    if (p.y >= 0) positive.y = bounding_box.max.y;
    if (p.z >= 0) positive.z = bounding_box.max.z;

    // If that vertex is outside, box is outside
    if (glm::dot(glm::vec3(p), positive) + p.w < 0) {
      return false;
    }
  }
  return true;
}
