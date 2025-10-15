#version 330 core
in vec2 TexCoords;               // pass screen-space UVs from vertex shader
out vec4 FragColor;

uniform sampler2D blurredScene;  // ping-pong blurred texture
uniform vec4 highlightColor;     // e.g. purple with alpha

void main() {
    // sample blurred scene where this block is drawn
    vec4 blurred = texture(blurredScene, TexCoords);

    // blend blurred scene with highlight tint
    FragColor = mix(blurred, highlightColor, highlightColor.a);
}
