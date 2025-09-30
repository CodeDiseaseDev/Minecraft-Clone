//
// Created by code on 9/28/25.
//

#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H
#include <glm/vec3.hpp>

struct lighting_config {
  glm::vec3 lightColor{1.0f, 1.0f, 0.8f};
  glm::vec3 sunDir{60, -50, 0};

  float sky_night_day_light_modifier = 1.0f;

  float ambientStrength = 0.35f;
  float diffuseStrength = 1.3f;
  float specularStrength = 0.2f;
  float shininess = 30.0f;
  float vibrancy = 2.0f;
  float contrast = 0.75f;
};

#endif //LIGHTING_CONFIG_H

