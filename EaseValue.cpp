//
// Created by code on 9/24/25.
//

#include "EaseValue.h"

void EaseValue::tick(float deltaTime, float multiplier) {
  value = LERP(
    value, target,
    deltaTime * multiplier
  );
}
