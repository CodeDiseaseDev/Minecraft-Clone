
#include "PlayerObject.h"

#include <stdexcept>

#include "PlayerObject.h"

#include <functional>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

// -----------------------------------------------------------
// Player part dimensions (in blocks)
// -----------------------------------------------------------
constexpr glm::vec3 PLAYER_TORSO_SIZE   = {0.5f, 0.75f, 0.25f};
constexpr glm::vec3 PLAYER_HEAD_SIZE    = {0.5f, 0.5f, 0.5f};
constexpr glm::vec3 PLAYER_ARM_SIZE     = {0.25f, 0.75f, 0.25f};
constexpr glm::vec3 PLAYER_LEG_SIZE     = {0.25f, 0.75f, 0.25f};

// -----------------------------------------------------------
// Offsets (relative to base at the player’s waist)
// -----------------------------------------------------------
constexpr float PLAYER_HEIGHT_OFFSET    = 1.075f; // lift body so feet touch ground
constexpr float PLAYER_HEAD_OFFSET_Y    = 0.625f;
constexpr float PLAYER_HEAD_PIVOT_Y     = 0.25f;
constexpr float PLAYER_ARM_OFFSET_X     = 0.375f;
constexpr float PLAYER_LEG_OFFSET_X     = 0.125f;
constexpr float PLAYER_LEG_OFFSET_Y     = -0.75f;

PlayerObject::PlayerObject(arena::Allocator<std::byte>& a, Shader* s, Texture* tex)
    : shader(s), texture_atlas(tex), arena(a)
{
    torso   = makeCubeMesh(PLAYER_TORSO_SIZE, [](SkinFaceIndex f){ return SkinUVRegistry::getTorsoUV(f); });
    head    = makeCubeMesh(PLAYER_HEAD_SIZE,  [](SkinFaceIndex f){ return SkinUVRegistry::getHeadUV(f); });
    leftArm = makeCubeMesh(PLAYER_ARM_SIZE,   [](SkinFaceIndex f){ return SkinUVRegistry::getArmUV(f); });
    rightArm= makeCubeMesh(PLAYER_ARM_SIZE,   [](SkinFaceIndex f){ return SkinUVRegistry::getArmUV(f); });
    leftLeg = makeCubeMesh(PLAYER_LEG_SIZE,   [](SkinFaceIndex f){ return SkinUVRegistry::getLegUV(f); });
    rightLeg= makeCubeMesh(PLAYER_LEG_SIZE,   [](SkinFaceIndex f){ return SkinUVRegistry::getLegUV(f); });
}

void PlayerObject::update(float dt) {
    // // Example walking animation
    // leftArmAngle  = sin(glfwGetTime() * 4.0f) * 0.5f;
    // rightArmAngle = -leftArmAngle;
    // leftLegAngle  = -leftArmAngle;
    // rightLegAngle = leftArmAngle;
}

void PlayerObject::draw(Camera& camera, ShadowMap* shadow_map) const {
    shader->use();
    shader->useCamera(camera);
    shader->useCameraLighting(camera);
    shader->useCameraWorldMesh(camera);

    glm::mat4 base = glm::translate(glm::mat4(1.0f),
                                    position + glm::vec3(0.0f, PLAYER_HEIGHT_OFFSET, 0.0f));

    glm::vec3 rotRad = glm::radians(rotation);
    base = base * glm::rotate(glm::mat4(1.0f), rotRad.x - glm::radians(0.0f), glm::vec3(0, 1, 0));

    // --- Torso ---
    {
        glm::mat4 model = base;
        shader->setMat4("model", model);
        torso->draw();
    }

    // --- Head ---
    {
        glm::mat4 model = base;
        model = model * glm::translate(glm::mat4(1.0f),
                                       glm::vec3(0.0f, PLAYER_HEAD_OFFSET_Y, 0.0f));
        model = model * glm::translate(glm::mat4(1.0f),
                                       glm::vec3(0.0f, -PLAYER_HEAD_PIVOT_Y, 0.0f));
        model = model * glm::rotate(glm::mat4(1.0f), -rotRad.y, glm::vec3(1, 0, 0));
        model = model * glm::translate(glm::mat4(1.0f),
                                       glm::vec3(0.0f, PLAYER_HEAD_PIVOT_Y, 0.0f));
        shader->setMat4("model", model);
        head->draw();
    }

    // --- Right Arm ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f),
                                                glm::vec3(PLAYER_ARM_OFFSET_X, 0, 0.0f));
        shader->setMat4("model", model);
        rightArm->draw();
    }

    // --- Left Arm ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f),
                                                glm::vec3(-PLAYER_ARM_OFFSET_X, 0, 0.0f));
        shader->setMat4("model", model);
        leftArm->draw();
    }

    // --- Left Leg ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f),
                                                glm::vec3(-PLAYER_LEG_OFFSET_X, PLAYER_LEG_OFFSET_Y, 0));
        shader->setMat4("model", model);
        leftLeg->draw();
    }

    // --- Right Leg ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f),
                                                glm::vec3(PLAYER_LEG_OFFSET_X, PLAYER_LEG_OFFSET_Y, 0));
        shader->setMat4("model", model);
        rightLeg->draw();
    }
}


Mesh* PlayerObject::makeCubeMesh(glm::vec3 size,
                                 std::function<FaceUV(SkinFaceIndex)> uvFunc)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 half = size * 0.5f;

    constexpr float PLAYER_ATLAS_WIDTH  = 256.0f;  // or whatever your sheet is
    constexpr float PLAYER_ATLAS_HEIGHT = 2048.0f;

    auto normalizeUV = [&](glm::vec2 p) -> glm::vec2 {
        return {
            p.x / PLAYER_ATLAS_WIDTH,
            1.0f - (p.y / PLAYER_ATLAS_HEIGHT) // flip to OpenGL origin
        };
    };

    struct Face {
        glm::vec3 normal;
        glm::vec3 v0, v1, v2, v3;
        SkinFaceIndex index;
    };

    std::vector<Face> faces = {
        { { 0,  0,  1}, { -half.x, -half.y,  half.z}, { half.x, -half.y,  half.z}, { half.x,  half.y,  half.z}, { -half.x,  half.y,  half.z}, SkinFaceIndex::POS_Z },
        { { 0,  0, -1}, {  half.x, -half.y, -half.z}, { -half.x, -half.y, -half.z}, { -half.x,  half.y, -half.z}, {  half.x,  half.y, -half.z}, SkinFaceIndex::NEG_Z },
        { {-1,  0,  0}, { -half.x, -half.y, -half.z}, { -half.x, -half.y,  half.z}, { -half.x,  half.y,  half.z}, { -half.x,  half.y, -half.z}, SkinFaceIndex::NEG_X },
        { { 1,  0,  0}, {  half.x, -half.y,  half.z}, {  half.x, -half.y, -half.z}, {  half.x,  half.y, -half.z}, {  half.x,  half.y,  half.z}, SkinFaceIndex::POS_X },
        { { 0,  1,  0}, { -half.x,  half.y,  half.z}, {  half.x,  half.y,  half.z}, {  half.x,  half.y, -half.z}, { -half.x,  half.y, -half.z}, SkinFaceIndex::POS_Y },
        { { 0, -1,  0}, { -half.x, -half.y, -half.z}, {  half.x, -half.y, -half.z}, {  half.x, -half.y,  half.z}, { -half.x, -half.y,  half.z}, SkinFaceIndex::NEG_Y },
    };

    for (auto& f : faces) {
        auto uv = uvFunc(f.index);
        unsigned int startIdx = vertices.size();

        std::array<glm::vec2, 4> texCoords = {
            normalizeUV(uv.uv0),
            normalizeUV(uv.uv1),
            normalizeUV(uv.uv2),
            normalizeUV(uv.uv3)
        };

        for (int v = 0; v < 4; ++v) {
            Vertex vert;
            vert.position = (v == 0 ? f.v0 : v == 1 ? f.v1 : v == 2 ? f.v2 : f.v3);
            vert.normal = f.normal;
            vert.texCoord = texCoords[v];
            vert.color = {1, 1, 1};
            vert.aoFactor = 1.0f;
            vert.bary = glm::vec3(0);
            vertices.push_back(vert);
        }

        indices.insert(indices.end(), {
            startIdx + 0, startIdx + 1, startIdx + 2,
            startIdx + 2, startIdx + 3, startIdx + 0
        });
    }

    return arena_allocate<Mesh>(arena, vertices, indices);
}


// Mesh* PlayerObject::makeCubeMesh(glm::vec3 size) {
//     std::vector<Vertex> vertices;
//     std::vector<unsigned int> indices;
//
//     // Half extents
//     glm::vec3 half = size * 0.5f;
//
//     // Define cube faces (positions and normals)
//     struct Face {
//         glm::vec3 normal;
//         glm::vec3 v0, v1, v2, v3;
//     };
//
//     std::vector<Face> faces = {
//         // Front (+Z)
//         { { 0,  0,  1}, { -half.x, -half.y,  half.z}, { half.x, -half.y,  half.z}, { half.x,  half.y,  half.z}, { -half.x,  half.y,  half.z} },
//         // Back (-Z)
//         { { 0,  0, -1}, {  half.x, -half.y, -half.z}, { -half.x, -half.y, -half.z}, { -half.x,  half.y, -half.z}, {  half.x,  half.y, -half.z} },
//         // Left (-X)
//         { {-1,  0,  0}, { -half.x, -half.y, -half.z}, { -half.x, -half.y,  half.z}, { -half.x,  half.y,  half.z}, { -half.x,  half.y, -half.z} },
//         // Right (+X)
//         { { 1,  0,  0}, {  half.x, -half.y,  half.z}, {  half.x, -half.y, -half.z}, {  half.x,  half.y, -half.z}, {  half.x,  half.y,  half.z} },
//         // Top (+Y)
//         { { 0,  1,  0}, { -half.x,  half.y,  half.z}, {  half.x,  half.y,  half.z}, {  half.x,  half.y, -half.z}, { -half.x,  half.y, -half.z} },
//         // Bottom (-Y)
//         { { 0, -1,  0}, { -half.x, -half.y, -half.z}, {  half.x, -half.y, -half.z}, {  half.x, -half.y,  half.z}, { -half.x, -half.y,  half.z} },
//     };
//
//     glm::vec2 uvs[4] = {
//         {0.0f, 0.0f},
//         {1.0f, 0.0f},
//         {1.0f, 1.0f},
//         {0.0f, 1.0f}
//     };
//
//     // Optional: simple color per face (can adjust as you like)
//     glm::vec3 faceColors[6] = {
//         {1, 0, 0}, // front - red
//         {0, 1, 0}, // back - green
//         {0, 0, 1}, // left - blue
//         {1, 1, 0}, // right - yellow
//         {1, 0, 1}, // top - magenta
//         {0, 1, 1}  // bottom - cyan
//     };
//
//     for (size_t i = 0; i < faces.size(); ++i) {
//         const Face& f = faces[i];
//         unsigned int startIdx = vertices.size();
//
//         for (int v = 0; v < 4; ++v) {
//             Vertex vert;
//             glm::vec3 pos;
//             switch (v) {
//                 case 0: pos = f.v0; break;
//                 case 1: pos = f.v1; break;
//                 case 2: pos = f.v2; break;
//                 case 3: pos = f.v3; break;
//             }
//
//             vert.position = pos;
//             vert.normal = f.normal;
//             vert.color = faceColors[i];
//             vert.texCoord = uvs[v];
//             vert.aoFactor = 1.0f;
//             vert.bary = glm::vec3(0.0f); // you can fill this if needed for wireframe shading
//
//             vertices.push_back(vert);
//         }
//
//         // Two triangles per face
//         indices.push_back(startIdx + 0);
//         indices.push_back(startIdx + 1);
//         indices.push_back(startIdx + 2);
//         indices.push_back(startIdx + 2);
//         indices.push_back(startIdx + 3);
//         indices.push_back(startIdx + 0);
//     }
//
//     // Allocate from arena
//     return arena_allocate<Mesh>(arena, vertices, indices);
// }

