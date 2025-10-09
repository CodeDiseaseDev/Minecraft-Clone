//
// Created by code on 9/28/25.
//

#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H
#include <glm/vec3.hpp>

struct lighting_config {
  glm::vec3 lightColor{1.0f, 1.0f, 1.0f};
  glm::vec3 sunDir{60, -35, 0};

  float sky_night_day_light_modifier = 1.0f;

  float ambientStrength = 0.495f;
  float diffuseStrength = 1.035f;
  float specularStrength = 0.0;
  float shininess = 0.0f;
  float vibrancy = 1.621f;
  float contrast = 0.884f;

  glm::vec3 fogColor{0.6f, 0.7f, 1.0f};
  float fogDensity = 0.03f;
  float fogStart = 50.0f;
  float fogEnd = 150.0f;

  int pcfRadius = 4;
  int bypassPostProcessing = 0;
  float ambientOcclusion = 0.95f;

  bool vsync = true;
};

#endif //LIGHTING_CONFIG_H

