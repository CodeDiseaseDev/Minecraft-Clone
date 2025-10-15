#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float ambientStrength  = 0.2;
uniform float diffuseStrength  = 0.8;
uniform float specularStrength = 0.5;
uniform float shininess        = 32.0;

// If you want solid colors for each body part
uniform vec4 _color;

// Optional texture (use atlas for skins later)
uniform sampler2D atlas;
uniform bool useTexture = false;

void main()
{
    // Lighting
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 ambient = ambientStrength * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    vec4 baseColor = _color;
    if (useTexture) {
        baseColor = texture(atlas, vTexCoord);
    }

    //FragColor = vec4(lighting, 1.0) * baseColor;
    FragColor = vec4(1,0,0,1);
}
