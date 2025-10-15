#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main() {
    vec4 clipPos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = clipPos;

    // Convert from clip space → normalized device coords → [0,1] UV
    TexCoords = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
}
