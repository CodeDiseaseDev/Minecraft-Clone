//
// Created by code on 9/24/25.
//

#include "EaseVec3.h"

void EaseVec3::tick(float deltaTime, float multiplier) {
  value.x = LERP(
    value.x, target.x,
    deltaTime * multiplier);

  value.y = LERP(
    value.y, target.y,
    deltaTime * multiplier);

  value.z = LERP(
    value.z, target.z,
    deltaTime * multiplier);
}
