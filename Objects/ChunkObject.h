//
// Created by code on 9/23/25.
//

#ifndef CHUNKOBJECT_H
#define CHUNKOBJECT_H
#include <memory>

#include "GameObject.h"
#include "ShadowMap.h"

#include "../Mesh.h"
#include "../Block.h"
#include "../Chunk.h"
#include "../Texture.h"
#include "../BlockRegistry.h"



const glm::vec3 faceVerts_PosX[4] = {
  {1, 0, 0},
  {1, 1, 0},
  {1, 1, 1},
  {1, 0, 1}
};

const glm::vec3 faceVerts_NegX[4] = {
  {0, 0, 1},
  {0, 1, 1},
  {0, 1, 0},
  {0, 0, 0}
};

const glm::vec3 faceVerts_PosY[4] = {
  {0, 1, 1},
  {1, 1, 1},
  {1, 1, 0},
  {0, 1, 0}
};

const glm::vec3 faceVerts_NegY[4] = {
  {0, 0, 0},
  {1, 0, 0},
  {1, 0, 1},
  {0, 0, 1}
};

const glm::vec3 faceVerts_PosZ[4] = {
  {0, 0, 1},
  {1, 0, 1},
  {1, 1, 1},
  {0, 1, 1}
};

const glm::vec3 faceVerts_NegZ[4] = {
  {0, 0, 0},
  {0, 1, 0},
  {1, 1, 0},
  {1, 0, 0}
};

// const glm::vec2 texCoords[4] = {
//   {0.0f, 0.0f},
//   {1.0f, 0.0f},
//   {1.0f, 1.0f},
//   {0.0f, 1.0f}
// };

// X faces
const glm::vec3 normal_PosX = { 1, 0, 0 };
const glm::vec3 normal_NegX = {-1, 0, 0 };

// Y faces
const glm::vec3 normal_PosY = { 0, 1, 0 };
const glm::vec3 normal_NegY = { 0,-1, 0 };

// Z faces
const glm::vec3 normal_PosZ = { 0, 0, 1 };
const glm::vec3 normal_NegZ = { 0, 0,-1 };


// Default quad UVs
const glm::vec2 texCoords[4] = {
  {0,0}, {1,0}, {1,1}, {0,1}
};

// Rotated 90° clockwise
const glm::vec2 texCoords_90[4] = {
  {1,0}, {1,1}, {0,1}, {0,0}
};

// Rotated 270° clockwise
const glm::vec2 texCoords_270[4] = {
  {0,1}, {0,0}, {1,0}, {1,1}
};

// Flipped vertically (for bottom faces)
const glm::vec2 texCoords_FlipV[4] = {
  {0,1}, {1,1}, {1,0}, {0,0}
};


class World;

class ChunkObject : public GameObject {
public:
  Mesh* chunk_mesh;


  Shader* shader;
  Texture* texture_atlas;
  Chunk* chunk;

  glm::vec3 chunk_pos {0,0,0};

  arena::Allocator<std::byte>& arena;


  explicit ChunkObject(
    arena::Allocator<std::byte>& arena,
    Shader* s,
    Texture* ta);

  // Called when world gen / block edits happen
  void rebuildMesh(World* world);

  void addFace(std::vector<Vertex>& vertices,
                          std::vector<unsigned int>& indices,
                          const glm::vec3& blockPos,
                          const glm::vec3 faceVerts[4],
                          const glm::vec3& normal,
                          const glm::vec2 texCoords[4],
                          glm::vec2 face_tile_uv, World* world);

  void Update(float dt) override;

  void draw(World* world, Camera& camera, ShadowMap* shadow_map);

  void setChunk(Chunk* chunk);

  AABB getChunkBoundingBox();

  static float computeAO(World* world, glm::ivec3 basePos,
                       glm::ivec3 side1, glm::ivec3 side2,
                       glm::ivec3 corner);

  ~ChunkObject();
};




#endif //CHUNKOBJECT_H
