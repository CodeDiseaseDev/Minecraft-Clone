//
// Created by code on 9/25/25.
//

#include "Texture.h"

#include <GL/glext.h>

#include "stb_image.h"
#include "glad/gl.h"
void Texture::load(const char *filename) {
  int width, height, channels;
  unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);

  if (!data) {
    printf("Failed to load texture '%s'\n", filename);
    return;
  }

  printf("Loaded '%s', %ix%i (%i channels)\n", filename, width, height, channels);

  this->width  = width;
  this->height = height;
  this->channels = channels;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Critical for PNGs

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, data);

  // Generate mipmaps
  glGenerateMipmap(GL_TEXTURE_2D);

  // Filtering (keep these, don’t overwrite them)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // or GL_LINEAR for smooth scaling

  // Wrap mode
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Anisotropic filtering
  float aniso = 0.0f;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

  stbi_image_free(data);
}


void Texture::bind(int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
}
