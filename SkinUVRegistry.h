//
// Created by code on 10/15/25.
//

#ifndef SKINUVREGISTRY_H
#define SKINUVREGISTRY_H



#pragma once
#include <glm/glm.hpp>
#include <array>

constexpr int default_skin_uv_x = 0,
              default_skin_uv_y = 1023;

enum class SkinFaceIndex {
  POS_X, NEG_X,
  POS_Y, NEG_Y,
  POS_Z, NEG_Z
};

struct FaceUV {
  glm::vec2 uv0;
  glm::vec2 uv1;
  glm::vec2 uv2;
  glm::vec2 uv3;
};

class SkinUVRegistry {
public:
  static FaceUV getHeadUV(SkinFaceIndex face);
  static FaceUV getTorsoUV(SkinFaceIndex face);
  static FaceUV getArmUV(SkinFaceIndex face);
  static FaceUV getLegUV(SkinFaceIndex face);

private:
  static FaceUV makeFace(float x, float y, float w, float h);
};




#endif //SKINUVREGISTRY_H
