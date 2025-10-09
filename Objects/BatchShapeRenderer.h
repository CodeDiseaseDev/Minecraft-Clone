#pragma once
#include "../Shader.h"
#include "../Camera.h"
#include "../Texture.h"
#include <glm/glm.hpp>
#include <vector>

class Texture;
class Shader;
class Camera;

struct BatchDrawItem {
  glm::vec2 pos;
  glm::vec2 size;
  float rot;
  glm::vec4 color;
  bool useTex;
  unsigned int texture_id;
  glm::vec4 uvRegion = {0, 0, 1, 1};
  glm::vec2 textureSize;
};

class BatchShapeRenderer {
public:
  explicit BatchShapeRenderer(Shader* shader);
  ~BatchShapeRenderer();

  void Start();
  void End(Camera& camera);

  // Drawing primitives
  void DrawBox(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
  void DrawCircle(const glm::vec2& pos, float radius, const glm::vec4& color);
  void DrawTexture(Texture* texture, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color = {1,1,1,1}, const glm::vec4& uvRegion = {0,0,1,1});
  void DrawTexture(unsigned int texture_id, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, const glm::vec4& uvRegion, const glm::vec2& textureSize);

private:
  Shader* shader;
  unsigned int VAO, VBO;
  std::vector<BatchDrawItem> items;
  // Texture* boundTexture = nullptr;
  bool started = false;

  void UploadUniforms();
};
