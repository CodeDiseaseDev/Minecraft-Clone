#include "GaussianBlurObject.h"
#include <glad/gl.h>

GaussianBlurObject::GaussianBlurObject(
    std::shared_ptr<Shader> &shaderH,
    std::shared_ptr<Shader> &shaderV,
    int width,
    int height)
    : blurShaderH(shaderH), blurShaderV(shaderV), width(width), height(height)
{
    initQuad();
    initPingpong();
}

void GaussianBlurObject::initQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void GaussianBlurObject::initPingpong() {
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongTex);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTex[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
                     GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, pingpongTex[i], 0);
    }
}

void GaussianBlurObject::updateSize(int newWidth, int newHeight) {
    // If size didn’t change, skip reallocation
    if (newWidth == width && newHeight == height)
        return;

    width = newWidth;
    height = newHeight;

    // Recreate textures for new resolution
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, pingpongTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
                     width, height, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // reset to default
}

unsigned int GaussianBlurObject::Apply(unsigned int inputTex, int iterations) {
    bool horizontal = true;
    bool firstIter = true;

    for (int i = 0; i < iterations; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glClear(GL_COLOR_BUFFER_BIT);

        auto &shader = horizontal ? blurShaderH : blurShaderV;
        shader->use();
        shader->setInt("image", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstIter ? inputTex : pingpongTex[!horizontal]);

        Draw();

        horizontal = !horizontal;
        if (firstIter) firstIter = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // return final blurred texture ID
    return pingpongTex[!horizontal];
}

void GaussianBlurObject::Draw() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
