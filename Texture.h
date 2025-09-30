//
// Created by code on 9/25/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "glad/gl.h"


class Texture {
  unsigned char* data;
public:
  GLuint id;


  int width, height, channels;

  void load(const char* filename);

  void bind(int unit = 0) const;
};



#endif //TEXTURE_H
