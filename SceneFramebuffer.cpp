#include "SceneFramebuffer.h"

SceneFramebuffer::SceneFramebuffer(int width, int height)
    : width(width), height(height)
{
    init(width, height);
}

SceneFramebuffer::~SceneFramebuffer() {
    cleanup();
}

void SceneFramebuffer::init(int w, int h) {
    // Create framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Color texture
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // Depth + stencil renderbuffer
    glGenRenderbuffers(1, &depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("SceneFramebuffer not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneFramebuffer::cleanup() {
    if (depthRBO) glDeleteRenderbuffers(1, &depthRBO);
    if (colorTex) glDeleteTextures(1, &colorTex);
    if (FBO) glDeleteFramebuffers(1, &FBO);
}

void SceneFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height); // ensure viewport matches
}

void SceneFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneFramebuffer::resize(int newWidth, int newHeight) {
    if (newWidth == width && newHeight == height) return;

    width = newWidth;
    height = newHeight;

    cleanup();
    init(width, height);
}

void SceneFramebuffer::blitToDefault(int winW, int winH) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);   // your FBO id
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, width, height,
        0, 0, winW, winH,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
