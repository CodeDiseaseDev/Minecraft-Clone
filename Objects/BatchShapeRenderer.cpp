#include "BatchShapeRenderer.h"
#include <string>
#include <algorithm>  // for std::min

BatchShapeRenderer::BatchShapeRenderer(Shader* shader)
    : shader(shader)
{
    // Quad (centered around origin)
    float quadVertices[] = {
        // pos       // uv
        -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f,  0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

BatchShapeRenderer::~BatchShapeRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void BatchShapeRenderer::Start() {
    items.clear();
    started = true;
}

void BatchShapeRenderer::End(Camera& camera) {
    if (!started || items.empty())
        return;

    shader->use();
    shader->setVec2("screenSize", camera.screenSize);
    glBindVertexArray(VAO);

    // Prepare render state (important!)
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw all items (simple per-item loop for now)
    for (int i = 0; i < items.size(); i++) {
        const auto& item = items[i];

        shader->setInt("uCount", 1);
        shader->setVec2("uPositions[0]", item.pos);
        shader->setVec2("uSizes[0]", item.size);
        shader->setFloat("uRotations[0]", item.rot);
        shader->setVec4("uColors[0]", item.color);
        shader->setInt("uUseTexture[0]", item.useTex);

        // Pass texture region + size
        shader->setVec4("uTexRegionPx", item.uvRegion);
        // shader->setVec2("uTextureSize", item.textureSize);

        if (item.useTex) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, item.texture_id);
            shader->setInt("uTexture", 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glBindVertexArray(0);
    glDisable(GL_BLEND);
    started = false;
}

void BatchShapeRenderer::DrawBox(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    items.push_back({pos, size, 0.0f, color, false});
}

void BatchShapeRenderer::DrawCircle(const glm::vec2& pos, float radius, const glm::vec4& color) {
    items.push_back({pos, {radius, radius}, 0.0f, color, false});
}

void BatchShapeRenderer::DrawTexture(
    Texture* texture,
    const glm::vec2& pos,
    const glm::vec2& size,
    const glm::vec4& color,
    const glm::vec4& uvRegion
) {
    items.push_back({
        pos,
        size,
        0.0f,
        color,
        true,
        texture->id,
        uvRegion,
        {texture->width, texture->height}
    });
}

void BatchShapeRenderer::DrawTexture(
    unsigned int texture_id,
    const glm::vec2& pos,
    const glm::vec2& size,
    const glm::vec4& color,
    const glm::vec4& uvRegion,
    const glm::vec2& textureSize
) {
    /*
        struct BatchDrawItem {
          glm::vec2 pos;
          glm::vec2 size;
          float rot;
          glm::vec4 color;
          bool useTex;
          unsigned int texture_id;
          glm::vec4 uvRegion = {0, 0, 1, 1};
          glm::vec2 textureSize;
        };
    */
    items.push_back({
        pos,
        size,
        0.0f,
        color,
        true,
        texture_id,
        uvRegion,
        textureSize
    });
}

void BatchShapeRenderer::UploadUniforms() {
    int count = std::min<int>(items.size(), 128);
    shader->setInt("uCount", count);

    for (int i = 0; i < count; i++) {
        shader->setVec2("uPositions[" + std::to_string(i) + "]", items[i].pos);
        shader->setVec2("uSizes[" + std::to_string(i) + "]", items[i].size);
        shader->setFloat("uRotations[" + std::to_string(i) + "]", items[i].rot);
        shader->setVec4("uColors[" + std::to_string(i) + "]", items[i].color);
        shader->setInt("uUseTexture[" + std::to_string(i) + "]", items[i].useTex);
    }
}
