//
// Created by code on 9/24/25.
//

#ifndef EASEVALUE_H
#define EASEVALUE_H
#include <glm/ext/quaternion_common.hpp>

#define LERP(a, b, t) (a + (b - a) * t)

class EaseValue {
public:
  float target = 0.0f;
  float value = 0.0f;

  void tick(
    float deltaTime,
    float multiplier = 1.0f);
};



#endif //EASEVALUE_H
