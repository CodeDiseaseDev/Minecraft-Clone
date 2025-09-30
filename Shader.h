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

#include "Camera.h"

class Shader {
public:
  unsigned int ID;

  // New: generic constructor (list of {src, type})
  Shader(const std::vector<std::pair<const char*, GLenum>>& sources, const char* name);

  // Old-style constructor (kept for backwards compat)
  Shader(const char* vertexSrc, const char* fragmentSrc, const char* name);

  void use() const;

  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string& name, const glm::vec3& v) const;
  void setVec2(const std::string& name, const glm::vec2& v) const;
  void setVec4(const std::string& name, const glm::vec4& v) const;
  void setMat4(const std::string &name, const glm::mat4& mat) const;
  void setInt(const std::string &name, const int value) const;

  void useCamera(Camera& camera) const;

  static std::shared_ptr<Shader> loadFromName(std::string name);

private:
  GLuint compile(GLenum type, const char* src);
};

#endif // SHADER_H
