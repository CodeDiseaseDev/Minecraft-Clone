//
// Created by code on 9/23/25.
//

#include "WorldRenderer.h"

#include <algorithm>

WorldRenderer::WorldRenderer(
  std::shared_ptr<Shader>& shaderPtr,
  std::shared_ptr<Texture>& ta,
  World &world)
  : shader(shaderPtr), texture_atlas(ta), world(world) {}

void WorldRenderer::rebuildChunkMesh(Camera &cam, Chunk *chunk) {
  (void)cam;
  if (chunk == nullptr) {
    return;
  }

  auto chunkObj = std::make_shared<ChunkObject>(shader, texture_atlas);
  chunkObj->setChunk(chunk);

  chunkObj->chunk_pos = {
    chunk->position.x * CHUNK_SIZE,
    chunk->position.y * CHUNK_SIZE,
    chunk->position.z * CHUNK_SIZE
  };

  chunkObj->rebuildMesh(world);
  visibleChunks.push_back(std::move(chunkObj));
}

void WorldRenderer::rebuildTheseChunks(
  Camera &cam,
  const std::vector<std::shared_ptr<Chunk>> &chunks) {

  for (const auto& chunk : chunks) {
    rebuildChunkMesh(cam, chunk.get());
  }
}

void WorldRenderer::rebuildTheseChunks(
  Camera &cam,
  const std::vector<Chunk *> &chunks) {

  for (auto* chunk : chunks) {
    rebuildChunkMesh(cam, chunk);
  }
}

void WorldRenderer::draw(Camera &cam, std::shared_ptr<ShadowMap> shadow_map) {

  this->rebuildTheseChunks(cam, chunksToRebuild);
  chunksToRebuild.clear();

  if (shader == nullptr) {
    throw std::runtime_error("Shader ptr is nullptr");
  }

  for (auto& chunkObj : visibleChunks) {
    chunkObj->draw(cam, shadow_map);
  }
}

void WorldRenderer::draw_depth_only(
  const glm::mat4 &lightSpaceMatrix,
  const std::shared_ptr<Shader> &depthShader) {

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  depthShader->use();
  depthShader->setMat4(
    "lightSpaceMatrix", lightSpaceMatrix);

  for (auto& chunk : visibleChunks) {
    if (!chunk->mesh->vao) continue;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, chunk->chunk_pos);
    depthShader->setMat4("model", model);

    chunk->mesh->draw();
    // glBindVertexArray(chunk->mesh->vao);
    // glDrawElements(GL_TRIANGLES, chunk->mesh->indexCount, GL_UNSIGNED_INT, 0);
  }

  // glBindVertexArray(0);
}

inline float distance2(const glm::vec3& a, const glm::vec3& b) {
  return glm::dot(a - b, a - b);
}

void WorldRenderer::tick(glm::vec3 player_location, int render_distance) {
  // for (auto& chunk : visibleChunks) {
  //   if (chunk == nullptr)
  //     continue;
  //
  //   float distance = glm::distance(
  //     chunk->position, player_location);
  //
  //   if (distance >= render_distance) {
  //     auto it = std::find_if(
  //         visibleChunks.begin(), visibleChunks.end(),
  //         [&](const std::shared_ptr<ChunkObject>& c) {
  //             return c->position == chunk->position; // compare raw pointer
  //         }
  //     );
  //
  //     if (it != visibleChunks.end()) {
  //       visibleChunks.erase(it);
  //     }
  //   }
  // }

  const float max_chunk_distance2 = (render_distance * CHUNK_SIZE) * (render_distance * CHUNK_SIZE);

  // std::erase_if(visibleChunks, [&](const auto& chunk) {
  //     if (!chunk) return true;
  //     float d2 = distance2(chunk->chunk_pos, player_location);
  //     return d2 >= max_chunk_distance2;
  // });

  // int removed = 0;
  // const int maxRemovals = 8; // tune this
  //
  // for (auto it = world.chunkColumns.begin(); it != world.chunkColumns.end() && removed < maxRemovals; ) {
  //   float dist2 = distance2(it->second.colCenter, player_location);
  //   if (dist2 >= max_chunk_distance2) {
  //     it = world.chunkColumns.erase(it);
  //     ++removed;
  //   } else {
  //     ++it;
  //   }
  // }


}
