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

void WorldRenderer::rebuildTheseChunks(
  Camera &cam,
  const std::vector<std::shared_ptr<Chunk>> &chunks) {

  for (auto& chunk : chunks) {
    auto chunkObj = std::make_shared<ChunkObject>(shader, texture_atlas);
    chunkObj->setChunk(chunk.get());

    chunkObj->chunk_pos = {
      chunk->position.x * CHUNK_SIZE,
      chunk->position.y * CHUNK_SIZE,
      chunk->position.z * CHUNK_SIZE
    };

    chunkObj->rebuildMesh();
    visibleChunks.push_back(chunkObj);
  }
}

void WorldRenderer::draw(Camera &cam, std::shared_ptr<ShadowMap> shadow_map) const {

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

  const float max_chunk_distance = static_cast<float>(render_distance * CHUNK_SIZE);

  visibleChunks.erase(
      std::remove_if(
          visibleChunks.begin(), visibleChunks.end(),
          [&](const std::shared_ptr<ChunkObject>& chunk) {
              if (!chunk) return true; // remove null chunks

              // Compare using world-space distances – chunk_pos is expressed in world units,
              // so we also work in world units here.
              const float distance = glm::distance(chunk->chunk_pos, player_location);
              return distance >= max_chunk_distance;
          }),
      visibleChunks.end()
  );

  for (auto it = world.chunkColumns.begin(); it != world.chunkColumns.end(); ) {
    glm::vec3 colCenter(
        it->first.x * CHUNK_SIZE + CHUNK_SIZE * 0.5f,
        player_location.y, // ignore height for distance
        it->first.y * CHUNK_SIZE + CHUNK_SIZE * 0.5f
    );

    float distance = glm::distance(colCenter, player_location);

    if (distance >= max_chunk_distance) {
      it = world.chunkColumns.erase(it);
    } else {
      ++it;
    }
  }


}
