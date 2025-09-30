//
// Created by code on 9/24/25.
//

#ifndef EASEVEC3_H
#define EASEVEC3_H
#include <glm/vec3.hpp>

#define LERP(a, b, t) (a + (b - a) * t)

class EaseVec3 {
public:
  glm::vec3 target{};
  glm::vec3 value{};

  void tick(
    float deltaTime,
    float multiplier = 1.0f);
};



#endif //EASEVEC3_H
