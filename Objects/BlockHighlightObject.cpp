//
// Created by code on 9/25/25.
//

#include "BlockHighlightObject.h"

BlockHighlightObject::BlockHighlightObject(arena::Allocator<std::byte>& arena, Shader* s)
    : shader(s)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Unit quad centered on origin, facing +Z
    const float half = 0.5f;

    glm::vec3 normal = {0, 0, 1};
    glm::vec3 positions[4] = {
        {-half, -half, 0.0f}, // bottom-left
        { half, -half, 0.0f}, // bottom-right
        { half,  half, 0.0f}, // top-right
        {-half,  half, 0.0f}, // top-left
    };

    for (auto& pos : positions)
        vertices.push_back({pos, normal});

    indices = {0, 1, 2, 0, 2, 3};
    mesh = arena_allocate<Mesh>(arena, vertices, indices);
}

void BlockHighlightObject::Draw(const RaycastHit& hit, Camera& camera, const glm::vec4& color)
{
    shader->use();
    shader->useCamera(camera);

    glm::mat4 model(1.0f);

    // Base position is the center of the voxel face hit
    glm::vec3 voxelCenter = glm::vec3(hit.voxel) + glm::vec3(0.5f);
    glm::vec3 faceOffset = glm::vec3(hit.normal) * 0.501f; // tiny offset to avoid z-fighting
    model = glm::translate(model, voxelCenter + faceOffset);

    // Orient the quad so that it faces the hit.normal
    if (hit.normal.x == 1)          // +X
        model = glm::rotate(model, glm::radians(90.0f), {0, 1, 0});
    else if (hit.normal.x == -1)    // -X
        model = glm::rotate(model, glm::radians(-90.0f), {0, 1, 0});
    else if (hit.normal.y == 1)     // +Y
        model = glm::rotate(model, glm::radians(-90.0f), {1, 0, 0});
    else if (hit.normal.y == -1)    // -Y
        model = glm::rotate(model, glm::radians(90.0f), {1, 0, 0});
    else if (hit.normal.z == -1)    // -Z
        model = glm::rotate(model, glm::radians(180.0f), {0, 1, 0});
    // +Z needs no rotation

    shader->setMat4("model", model);
    shader->setVec4("highlightColor", color);

    glDisable(GL_CULL_FACE);               // make both sides visible
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    mesh->draw();
    glEnable(GL_CULL_FACE);
}
