//
// ShadowMap.cpp
//
#include "ShadowMap.h"

#include <cmath>

ShadowMap::ShadowMap(std::shared_ptr<Shader>& depthShader, int res)
    : resolution(res), shader(depthShader) {
    Init(resolution);
}

ShadowMap::~ShadowMap() {
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
}

void ShadowMap::Init(int res) {
    if (depthMapFBO) glDeleteFramebuffers(1, &depthMapFBO);
    if (depthMap) glDeleteTextures(1, &depthMap);

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
             res, res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bind(int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, GetDepthMap());
}

void ShadowMap::Resize(int newRes) {
    if (newRes == resolution) return;
    resolution = newRes;
    Init(newRes);
}

glm::mat4 ShadowMap::GetLightSpaceMatrix(const glm::vec3& sunDir, const glm::vec3& center) {
    float near_plane = 1.0f, far_plane = 300.0f;
    float orthoSize = 100.0f; // cover your scene

    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize,
                                           -orthoSize, orthoSize,
                                           near_plane, far_plane);

    glm::vec3 dir = glm::length(sunDir) > 0.0001f ? glm::normalize(sunDir)
                                                  : glm::vec3(0.0f, -1.0f, 0.0f);
    float lightDistance = 100.0f;
    lastLightPos_ = center - dir * lightDistance; // place light opposite the sunDir

    glm::vec3 up = std::abs(glm::dot(dir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
                        ? glm::vec3(0.0f, 0.0f, 1.0f)
                        : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 lightView = glm::lookAt(lastLightPos_, center, up);

    return lightProjection * lightView;
}


// Camera ShadowMap::GetLightCamera(const glm::vec3& sunDir) const {
//     Camera lightCam(1.0f); // aspect = 1.0 since depth map is square
//
//     // Place the light far back along sunDir
//     float distance = 100.0f;
//     lightCam.position = -sunDir * distance;
//     lightCam.rotation = glm::vec3(0.0f); // not really used, since we'll compute view from position+direction
//
//     // Ortho projection for sun light
//     float orthoSize = 80.0f;
//     lightCam.fov = 0.0f; // not used
//     lightCam.aspect = 1.0f;
//     lightCam.nearPlane = 0.1f;
//     lightCam.farPlane  = 200.0f;
//
//     // Instead of perspective, override projection
//     // you can add this to your Camera class:
//     // glm::mat4 getOrthoMatrix(float left, float right, float bottom, float top, float near, float far) const;
//     // OR, just compute in ShadowMap
//     return lightCam;
// }

void ShadowMap::BeginDepthPass(glm::mat4 lightSpaceMatrix) {
    lastLightSpace_ = lightSpaceMatrix;

    shader->use();
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);


    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::EndDepthPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
