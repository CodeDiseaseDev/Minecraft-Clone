
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

    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);
    base = glm::rotate(base, rotation.y, glm::vec3(0,1,0));
    //
    // texture_atlas->bind(0);
    // shader->setInt("atlas", 0);

    // if (shadow_map) {
    //     shader->setMat4("lightSpaceMatrix", shadow_map->LastLightSpace());
    //     shader->setVec3("lightPos", shadow_map->LastLightPosition());
    //     shadow_map->bind(1);
    // }
    // shader->setInt("shadowMap", 1);

    // Draw torso
    glm::mat4 torsoModel = base;
    shader->setMat4("model", torsoModel);
    torso->draw();

    // Draw head (offset upward)
    glm::mat4 headModel = torsoModel * glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.2f, 0));
    shader->setMat4("model", headModel);
    head->draw();

    // Arms
    glm::mat4 lArmModel = torsoModel
        * glm::translate(glm::mat4(1.0f), glm::vec3(-0.65f, 1.0f, 0))
        * glm::rotate(glm::mat4(1.0f), leftArmAngle, glm::vec3(1,0,0))
        * glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.6f, 0));
    shader->setMat4("model", lArmModel);
    leftArm->draw();

    glm::mat4 rArmModel = torsoModel
        * glm::translate(glm::mat4(1.0f), glm::vec3(0.65f, 1.0f, 0))
        * glm::rotate(glm::mat4(1.0f), rightArmAngle, glm::vec3(1,0,0))
        * glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.6f, 0));
    shader->setMat4("model", rArmModel);
    rightArm->draw();

    // Legs
    glm::mat4 lLegModel = torsoModel
        * glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f, -1.2f, 0))
        * glm::rotate(glm::mat4(1.0f), leftLegAngle, glm::vec3(1,0,0))
        * glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.6f, 0));
    shader->setMat4("model", lLegModel);
    leftLeg->draw();

    glm::mat4 rLegModel = torsoModel
        * glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, -1.2f, 0))
        * glm::rotate(glm::mat4(1.0f), rightLegAngle, glm::vec3(1,0,0))
        * glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.6f, 0));
    shader->setMat4("model", rLegModel);
    rightLeg->draw();
}

Mesh* PlayerObject::makeCubeMesh(glm::vec3 size) {
    // For simplicity: return a unit cube scaled to "size"
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // TODO: build vertices for a cube with dimensions `size.x, size.y, size.z`
    // You can reuse your existing "addFace" logic from ChunkObject.

    return arena_allocate<Mesh>(arena, vertices, indices);
}

