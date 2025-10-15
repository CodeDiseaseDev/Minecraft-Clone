
#include "PlayerObject.h"

#include <stdexcept>

#include "PlayerObject.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

PlayerObject::PlayerObject(arena::Allocator<std::byte>& a, Shader* s, Texture* tex)
    : shader(s), texture_atlas(tex), arena(a)
{
    // Sizes are roughly Minecraft proportions (in blocks)
    torso   = makeCubeMesh({0.8f, 1.2f, 0.4f});
    head    = makeCubeMesh({0.9f, 0.9f, 0.9f});
    leftArm = makeCubeMesh({0.3f, 1.2f, 0.3f});
    rightArm= makeCubeMesh({0.3f, 1.2f, 0.3f});
    leftLeg = makeCubeMesh({0.4f, 1.2f, 0.4f});
    rightLeg= makeCubeMesh({0.4f, 1.2f, 0.4f});
}

void PlayerObject::update(float dt) {
    // Example walking animation
    leftArmAngle  = sin(glfwGetTime() * 4.0f) * 0.5f;
    rightArmAngle = -leftArmAngle;
    leftLegAngle  = -leftArmAngle;
    rightLegAngle = leftArmAngle;
}

void PlayerObject::draw(Camera& camera, ShadowMap* shadow_map) const {
    shader->use();
    shader->useCamera(camera);
    shader->useCameraLighting(camera);
    shader->useCameraWorldMesh(camera);

    // Move everything down by 3.6 so the legs sit on ground
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position + glm::vec3(0.0f, 1.8f, 0.0f));


    glm::vec3 rotRad = glm::radians(rotation);
    base = base * glm::rotate(glm::mat4(1.0f), -rotRad.x - 67.5f, glm::vec3(0, 1, 0));

    // --- Torso ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0, 0.0f));
        shader->setMat4("model", model);
        torso->draw();
    }

    // --- Head ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.05f, 0.0f));
        model = model * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.45f, 0.0f));
        model = model * glm::rotate(glm::mat4(1.0f), -rotRad.y, glm::vec3(1, 0, 0));
        model = model * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.45f, 0.0f));
        shader->setMat4("model", model);
        head->draw();
    }

    // --- Right Arm ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0.55f, 0, 0.0f));
        shader->setMat4("model", model);
        rightArm->draw();
    }

    // --- Left Arm ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.55f, 0, 0.0f));
        shader->setMat4("model", model);
        leftArm->draw();
    }

    // --- Left Leg ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, 0 - 1.2f, 0));
        shader->setMat4("model", model);
        leftLeg->draw();
    }

    // --- Right Leg ---
    {
        glm::mat4 model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0 - 1.2f, 0));
        shader->setMat4("model", model);
        rightLeg->draw();
    }
}





Mesh* PlayerObject::makeCubeMesh(glm::vec3 size) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Half extents
    glm::vec3 half = size * 0.5f;

    // Define cube faces (positions and normals)
    struct Face {
        glm::vec3 normal;
        glm::vec3 v0, v1, v2, v3;
    };

    std::vector<Face> faces = {
        // Front (+Z)
        { { 0,  0,  1}, { -half.x, -half.y,  half.z}, { half.x, -half.y,  half.z}, { half.x,  half.y,  half.z}, { -half.x,  half.y,  half.z} },
        // Back (-Z)
        { { 0,  0, -1}, {  half.x, -half.y, -half.z}, { -half.x, -half.y, -half.z}, { -half.x,  half.y, -half.z}, {  half.x,  half.y, -half.z} },
        // Left (-X)
        { {-1,  0,  0}, { -half.x, -half.y, -half.z}, { -half.x, -half.y,  half.z}, { -half.x,  half.y,  half.z}, { -half.x,  half.y, -half.z} },
        // Right (+X)
        { { 1,  0,  0}, {  half.x, -half.y,  half.z}, {  half.x, -half.y, -half.z}, {  half.x,  half.y, -half.z}, {  half.x,  half.y,  half.z} },
        // Top (+Y)
        { { 0,  1,  0}, { -half.x,  half.y,  half.z}, {  half.x,  half.y,  half.z}, {  half.x,  half.y, -half.z}, { -half.x,  half.y, -half.z} },
        // Bottom (-Y)
        { { 0, -1,  0}, { -half.x, -half.y, -half.z}, {  half.x, -half.y, -half.z}, {  half.x, -half.y,  half.z}, { -half.x, -half.y,  half.z} },
    };

    glm::vec2 uvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    // Optional: simple color per face (can adjust as you like)
    glm::vec3 faceColors[6] = {
        {1, 0, 0}, // front - red
        {0, 1, 0}, // back - green
        {0, 0, 1}, // left - blue
        {1, 1, 0}, // right - yellow
        {1, 0, 1}, // top - magenta
        {0, 1, 1}  // bottom - cyan
    };

    for (size_t i = 0; i < faces.size(); ++i) {
        const Face& f = faces[i];
        unsigned int startIdx = vertices.size();

        for (int v = 0; v < 4; ++v) {
            Vertex vert;
            glm::vec3 pos;
            switch (v) {
                case 0: pos = f.v0; break;
                case 1: pos = f.v1; break;
                case 2: pos = f.v2; break;
                case 3: pos = f.v3; break;
            }

            vert.position = pos;
            vert.normal = f.normal;
            vert.color = faceColors[i];
            vert.texCoord = uvs[v];
            vert.aoFactor = 1.0f;
            vert.bary = glm::vec3(0.0f); // you can fill this if needed for wireframe shading

            vertices.push_back(vert);
        }

        // Two triangles per face
        indices.push_back(startIdx + 0);
        indices.push_back(startIdx + 1);
        indices.push_back(startIdx + 2);
        indices.push_back(startIdx + 2);
        indices.push_back(startIdx + 3);
        indices.push_back(startIdx + 0);
    }

    // Allocate from arena
    return arena_allocate<Mesh>(arena, vertices, indices);
}

