//
// Created by code on 9/25/25.
//

#include "Texture.h"

#include "stb_image.h"
#include "glad/gl.h"

void Texture::load(const char *filename) {
  // // glGenTextures(1, &atlasTex);
  // // glBindTexture(GL_TEXTURE_2D, atlasTex);
  //
  // int width, height, channels;
  // stbi_set_flip_vertically_on_load(true);
  // unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
  //
  // printf("Loaded '%s', %ix%i\n", filename, width, height);
  //
  // this->width  = width;
  // this->height = height;
  // this->channels = channels;
  //
  // glGenTextures(1, &id);
  // glBindTexture(GL_TEXTURE_2D, id);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);
  //
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //
  //
  // stbi_image_free(data);


  int width, height, channels;
  // stbi_set_flip_vertically_on_load(true); // Fix orientation
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

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(data);
}

void Texture::bind(int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
}
