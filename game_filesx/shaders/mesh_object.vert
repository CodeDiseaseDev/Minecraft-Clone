#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aAO;
layout (location = 4) in vec4 aColor;


out float vAO;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;
out vec4 FragPosLightSpace;
out vec4 vertColor;

uniform mat4 model, view, projection, lightSpaceMatrix;

void main() {
    vTexCoord = aTexCoord;
    vNormal   = mat3(transpose(inverse(model))) * aNormal;
    vec4 worldPos = model * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    gl_Position = projection * view * worldPos;
    vAO = aAO;

    vertColor = aColor;
}
