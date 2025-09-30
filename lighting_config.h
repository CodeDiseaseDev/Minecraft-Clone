//
// Created by code on 9/28/25.
//

#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H
#include <glm/vec3.hpp>

struct lighting_config {
  glm::vec3 lightColor{1.0f, 1.0f, 0.8f};
  glm::vec3 sunDir{30, -90, 0};

  float sky_night_day_light_modifier = 0.5f;

  float ambientStrength = 0.4f;
  float diffuseStrength = 1.0f;
  float specularStrength = 0.15f;
  float shininess = 60.0f;
  float vibrancy = 1.3f;
  float contrast = 0.85f;
};

#endif //LIGHTING_CONFIG_H

