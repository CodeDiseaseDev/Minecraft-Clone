//
// Created by code on 9/23/25.
//

#include "ChunkObject.h"


ChunkObject::ChunkObject(std::shared_ptr<Shader>& s,
    std::shared_ptr<Texture>& ta):
  shader(s), texture_atlas(ta) {}



void ChunkObject::rebuildMesh() {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  if (chunk == nullptr) {
    throw std::runtime_error("Chunk ptr is nullptr");
  }

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        auto& block = chunk->get(x, y, z);
        if (block.isAir()) continue;

        glm::vec3 pos(x, y, z);

        // +X face
        if (x == CHUNK_SIZE-1 || chunk->isAir(x + 1, y, z))
          addFace(vertices, indices, pos, faceVerts_PosX, normal_PosX, texCoords_270,  3,25);

        // -X face
        if (x == 0 || chunk->isAir(x - 1, y, z))
          addFace(vertices, indices, pos, faceVerts_NegX, normal_NegX, texCoords_270,  3,25);

        // +Y face (top)
        if (y == CHUNK_SIZE-1 || chunk->isAir(x, y + 1, z))
          addFace(vertices, indices, pos, faceVerts_PosY, normal_PosY, texCoords,  6,25);

        // -Y face (bottom)
        if (y == 0 || chunk->isAir(x, y - 1, z))
          addFace(vertices, indices, pos, faceVerts_NegY, normal_NegY, texCoords_FlipV,  0,21);

        // +Z face (front)
        if (z == CHUNK_SIZE-1 || chunk->isAir(x, y, z + 1))
          addFace(vertices, indices, pos, faceVerts_PosZ, normal_PosZ, texCoords_FlipV,  3,25);

        // -Z face (back)
        if (z == 0 || chunk->isAir(x, y, z - 1))
          addFace(vertices, indices, pos, faceVerts_NegZ, normal_NegZ, texCoords_270,  3,25);
      }
    }
  }

  mesh = std::make_unique<Mesh>(vertices, indices);
}


glm::vec2 getUVForTile(int tileX, int tileY, glm::vec2 localUV) {
  const float TILE_SIZE = 1.0f / 16.0f; // if 16 tiles across
  return glm::vec2(
      (tileX + localUV.x) * TILE_SIZE,
      (tileY + localUV.y) * TILE_SIZE
  );
}

void ChunkObject::addFace(std::vector<Vertex>& vertices,
                          std::vector<unsigned int>& indices,
                          const glm::vec3& blockPos,
                          const glm::vec3 faceVerts[4],
                          const glm::vec3& normal,
                          const glm::vec2 texCoords[4],
                          int tileX, int tileY)
{
  unsigned int startIndex = vertices.size();
  glm::vec3 color = {0.2f, 0.2f, 0.2f};

  // Atlas info
  const float atlasWidth  = 256.0f;
  const float atlasHeight = 1024.0f;
  const float tileSize    = 16.0f;

  // Compute UV bounds for this tile
  float u0 = (tileX * tileSize) / atlasWidth;
  float v0 = (tileY * tileSize) / atlasHeight;
  float u1 = ((tileX + 1) * tileSize) / atlasWidth;
  float v1 = ((tileY + 1) * tileSize) / atlasHeight;

  auto makeVertex = [&](int i, glm::vec3 bary) {
    Vertex v;
    v.position = blockPos + faceVerts[i];
    v.normal   = normal;
    v.color    = color;

    glm::vec2 localUV = texCoords[i]; // (0,0),(1,0),(1,1),(0,1)
    v.texCoord = {
      u0 + localUV.x * (u1 - u0),
      v0 + localUV.y * (v1 - v0)
    };

    v.bary = bary;
    return v;
  };

  vertices.push_back(makeVertex(0, {1,0,0}));
  vertices.push_back(makeVertex(1, {0,1,0}));
  vertices.push_back(makeVertex(2, {0,0,1}));
  vertices.push_back(makeVertex(3, {0,1,0}));

  indices.push_back(startIndex + 0);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex + 0);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}




void ChunkObject::Update(float dt) {
  // chunks usually don’t need much here
}

void ChunkObject::draw(Camera &camera, std::shared_ptr<ShadowMap> shadow_map) const {


  if (shader == nullptr) {
    throw std::runtime_error("Shader ptr is nullptr");
  }

  shader->use();
  shader->useCamera(camera);

  shader->setMat4("model", glm::translate(glm::mat4(1.0f), chunk_pos));
  shader->setVec4("_color", glm::vec4(0.8f, 0.8f, 0.8f, 1));


  texture_atlas->bind(0);
  shader->setInt("atlas", 0);

  if (shadow_map) {
    shader->setMat4("lightSpaceMatrix", shadow_map->LastLightSpace());
    shader->setVec3("lightPos", shadow_map->LastLightPosition());
    shadow_map->bind(1);
  } else {
    shader->setMat4("lightSpaceMatrix", glm::mat4(1.0f));
    shader->setVec3("lightPos", glm::vec3(0.0f));
  }
  shader->setInt("shadowMap", 1);

  mesh->draw();
}


void ChunkObject::setChunk(Chunk* chunk) {
  this->chunk = chunk;
}

AABB ChunkObject::getChunkBoundingBox() {
  glm::vec3 min = chunk_pos;             // already world coords
  glm::vec3 max = min + glm::vec3(CHUNK_SIZE);

  return {min, max};
}



