//
// Created by code on 9/23/25.
//

#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H
#include <memory>
#include <vector>

#include "World.h"
#include "Objects/ChunkObject.h"
#include "Objects/ShadowMap.h"

inline const std::array<std::tuple<int,int,int>, 6> neighborOffsets = {{
  {  1,  0,  0 }, // +X
  { -1,  0,  0 }, // -X
  {  0,  1,  0 }, // +Y
  {  0, -1,  0 }, // -Y
  {  0,  0,  1 }, // +Z
  {  0,  0, -1 }  // -Z
}};


class WorldRenderer {
public:
  std::shared_ptr<Shader> &shader;
  std::vector<std::shared_ptr<ChunkObject>> visibleChunks;

  std::vector<std::shared_ptr<Chunk>> chunksToRebuild;

  std::shared_ptr<Texture>& texture_atlas;
  World &world;


  WorldRenderer(
    std::shared_ptr<Shader>& shader,
    std::shared_ptr<Texture>& ta,
    World &world);

  // void rebuildVisibleChunks(World& world, Camera& cam);

  void rebuildTheseChunks(
    Camera& cam,
    const std::vector<std::shared_ptr<Chunk>>& chunks);

  void draw(Camera& cam, std::shared_ptr<ShadowMap> shadow_map);

  void draw_depth_only(const glm::mat4& lightSpaceMatrix, const std::shared_ptr<Shader>& depthShader);

  void tick(glm::vec3 player_location, int render_distance);
};



#endif //CHUNKRENDERER_H
