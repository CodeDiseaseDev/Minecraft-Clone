//
// Created by code on 9/23/25.
//

#include "ChunkObject.h"
#include "../World.h"


ChunkObject::ChunkObject(Shader* s,
    Texture* ta):
  shader(s), texture_atlas(ta) {}




void ChunkObject::rebuildMesh(World* world) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  if (!chunk) {
    throw std::runtime_error("Chunk ptr is nullptr");
  }

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        const Block& block = chunk->get(x, y, z);
        if (block.isAir()) continue;

        // world position of this block (Y is up)
        glm::ivec3 wp = chunk->toWorldCoords(x, y, z);
        glm::vec3 lp(x, y, z); // local position for vertices

        // +X
        if (world->isAir(wp.x + 1, wp.y, wp.z)) {
          addFace(vertices, indices, lp,
            faceVerts_PosX, normal_PosX, texCoords_270,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::POS_X], world);
        }

        // -X
        if (world->isAir(wp.x - 1, wp.y, wp.z)) {
          addFace(vertices, indices, lp,
            faceVerts_NegX, normal_NegX, texCoords_270,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::NEG_X], world);
        }

        // +Y (top)
        if (world->isAir(wp.x, wp.y + 1, wp.z)) {
          addFace(vertices, indices, lp,
            faceVerts_PosY, normal_PosY, texCoords,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::POS_Y], world);
        }

        // -Y (bottom)
        if (world->isAir(wp.x, wp.y - 1, wp.z)) {
          addFace(vertices, indices, lp,
            faceVerts_NegY, normal_NegY, texCoords_FlipV,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::NEG_Y], world);
        }

        // +Z (front)
        if (world->isAir(wp.x, wp.y, wp.z + 1)) {
          addFace(vertices, indices, lp,
            faceVerts_PosZ, normal_PosZ, texCoords_FlipV,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::POS_Z], world);
        }

        // -Z (back)
        if (world->isAir(wp.x, wp.y, wp.z - 1)) {
          addFace(vertices, indices, lp,
            faceVerts_NegZ, normal_NegZ, texCoords_270,
            BlockRegistry::BlockUVs[block.id].faces[FaceIndex::NEG_Z], world);
        }
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
                          glm::vec2 face_tile_uv, World* world)
{
  unsigned int startIndex = vertices.size();
  glm::vec3 color = {0.2f, 0.2f, 0.2f};

  // Atlas info
  const float atlasWidth  = 256.0f;
  const float atlasHeight = 1024.0f;
  const float tileSize    = 16.0f;

  // Compute UV bounds for this tile
  int tileX = face_tile_uv.x;
  int tileY = face_tile_uv.y;
  float u0 = (tileX * tileSize) / atlasWidth;
  float v0 = (tileY * tileSize) / atlasHeight;
  float u1 = ((tileX + 1) * tileSize) / atlasWidth;
  float v1 = ((tileY + 1) * tileSize) / atlasHeight;

  auto makeVertex = [&](int i, glm::vec3 bary) {
    Vertex v;
    v.position = blockPos + faceVerts[i] - 0.5f;
    v.normal   = normal;
    v.color    = color;

    glm::vec2 localUV = texCoords[i];
    v.texCoord = {
      u0 + localUV.x * (u1 - u0),
      v0 + localUV.y * (v1 - v0)
    };
    v.bary = bary;

    // --- AO: robust per-corner sampling ---
    // Block world coords (of this block)
    glm::ivec3 wp = this->chunk->toWorldCoords(
        (int)blockPos.x, (int)blockPos.y, (int)blockPos.z);

    // Step one cell OUTSIDE the face (so we test neighbors that occlude this face)
    glm::ivec3 nstep = glm::ivec3(
        (int)glm::round(normal.x),
        (int)glm::round(normal.y),
        (int)glm::round(normal.z)
    );
    glm::ivec3 base = wp + nstep; // <- critical: outside the face

    // Corner signs from the actual corner coordinates (0 or 1)
    const int sx = (faceVerts[i].x > 0.5f) ? +1 : -1;
    const int sy = (faceVerts[i].y > 0.5f) ? +1 : -1;
    const int sz = (faceVerts[i].z > 0.5f) ? +1 : -1;

    // Choose the two tangent directions automatically
    glm::ivec3 side1(0), side2(0);
    if (nstep.x != 0) {            // ±X face -> tangents are Y and Z
      side1 = {0, sy, 0};
      side2 = {0, 0, sz};
    } else if (nstep.y != 0) {     // ±Y face -> tangents are X and Z
      side1 = {sx, 0, 0};
      side2 = {0, 0, sz};
    } else {                       // ±Z face -> tangents are X and Y
      side1 = {sx, 0, 0};
      side2 = {0, sy, 0};
    }

    v.aoFactor = computeAO(world, base, side1, side2, side1 + side2);

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

void ChunkObject::draw(Camera &camera, ShadowMap* shadow_map) const {


  if (shader == nullptr) {
    throw std::runtime_error("Shader ptr is nullptr");
  }

  shader->use();
  shader->useCamera(camera);
  shader->useCameraWorldMesh(camera);
  shader->useCameraLighting(camera);

  shader->setMat4("model", glm::translate(glm::mat4(1.0f), chunk_pos));
  // shader->setVec4("_color", glm::vec4(0.8f, 0.8f, 0.8f, 1));




  texture_atlas->bind(0);
  shader->setInt("atlas", 0);
  shader->setInt("useAtlas", 1);

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


// Calculate AO factor for a vertex given its adjacent block states
float ChunkObject::computeAO(World* world, glm::ivec3 basePos,
                             glm::ivec3 side1, glm::ivec3 side2,
                             glm::ivec3 corner)
{
  bool s1 = !world->isAir(basePos + side1);
  bool s2 = !world->isAir(basePos + side2);
  bool c  = !world->isAir(basePos + corner);

  int occ = (s1 && s2) ? 3 : (int)s1 + (int)s2 + (int)c;
  float ao = 1.0f - (occ / 3.0f);

  // optional: gentle bias so corners aren’t pitch black
  return glm::clamp(ao * 0.9f + 0.1f, 0.0f, 1.0f);
}





