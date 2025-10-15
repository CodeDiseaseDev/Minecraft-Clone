#version 330 core
in vec2 vTexCoord;
in vec4 vColor;
in float vUseTexture;

out vec4 FragColor;
uniform sampler2D uTexture;



void main()
{
    vec2 uv = vec2(vTexCoord.x, 1.0 - vTexCoord.y);
    vec4 texColor = texture(uTexture, uv);

    vec4 base = mix(vec4(1.0), texColor, vUseTexture); // if useTexture=0 → pure color
    FragColor = base * vColor;
}

