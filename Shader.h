#ifndef SHADER_H
#define SHADER_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "arena_alloc.h"
#include "Camera.h"

class Shader {
public:
  unsigned int ID;
  std::string shader_name;

  // New: generic constructor (list of {src, type})
  Shader(const std::vector<std::pair<const char*, GLenum>>& sources, const char* name);

  // Old-style constructor (kept for backwards compat)
  Shader(const char* vertexSrc, const char* fragmentSrc, const char* name);

  void use() const;

  bool isValid() const;

  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string& name, const glm::vec3& v) const;
  void setVec2(const std::string& name, const glm::vec2& v) const;
  void setVec4(const std::string& name, const glm::vec4& v) const;
  void setMat4(const std::string &name, const glm::mat4& mat) const;
  void setInt(const std::string &name, const int value) const;

  void useCamera(Camera& camera) const;
  void useCameraWorldMesh(Camera& camera);
  void useCameraLighting(Camera& camera);

  // void setScreenSize(int width, int height) const;

  void uniformNotFound(const std::string& name) const;

  void reload();

  static Shader* loadFromName(const std::string name, arena::Allocator<std::byte>& allocator);

  ~Shader();

private:
  GLuint compile(GLenum type, const char* src);
};

#endif // SHADER_H
