#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 FragPosLightSpace;
in float vAO;
in vec4 vertColor;

out vec4 FragColor;

uniform int useAtlas;
uniform sampler2D atlas;
uniform sampler2D shadowMap;

// --- Lighting uniforms ---
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

// --- Visual grading ---
uniform float vibrancy;
uniform float contrast;
uniform float ambientOcclusion;

// --- Fog ---
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

// --- Shadow ---
uniform int pcfRadius;

// --- NEW ---
uniform int noPostProcessing; // 1 = disable lighting, fog, AO, vibrancy, contrast

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = 0.0;

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int samples = 0;

    for (int x = -pcfRadius; x <= pcfRadius; ++x)
    for (int y = -pcfRadius; y <= pcfRadius; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
        samples++;
    }

    return shadow / float(samples);
}

void main()
{
    // --- Base texture ---
    vec3 texColor = pow(vertColor.rgb, vec3(2.2)); // sRGB → linear
    if (useAtlas == 1)
        texColor = texture(atlas, vTexCoord).rgb;

    // --- Skip everything if postprocessing is disabled ---
    if (noPostProcessing == 1) {
        FragColor = vec4(texColor, 1.0);
        return;
    }

    // --- Apply visual grading BEFORE lighting ---
    float avg = (texColor.r + texColor.g + texColor.b) / 3.0;
    texColor = mix(vec3(avg), texColor, vibrancy);
    texColor = (texColor - 0.5) * contrast + 0.5;

    // --- Lighting ---
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    vec3 viewDir   = normalize(viewPos - vFragPos);
    vec3 reflectDir= reflect(-lightDir, norm);
    float spec     = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular  = specularStrength * spec * lightColor;

    vec3 ambient   = ambientStrength * lightColor;

    float shadow   = ShadowCalculation(FragPosLightSpace);
    vec3 lighting  = (ambient + (1.0 - shadow) * (diffuse + specular));

    // Apply lighting to base color
    vec3 litColor = texColor * lighting;

    // --- AO + fog ---
    litColor *= mix(1.0 - ambientOcclusion, 1.0, vAO);

    float dist = length(viewPos - vFragPos);
    float fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);

    vec3 finalColor = mix(fogColor, litColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
