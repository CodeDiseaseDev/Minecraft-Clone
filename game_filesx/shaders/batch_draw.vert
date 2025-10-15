#version 330 core

layout (location = 0) in vec2 aPos;       // local quad vertex (0..1)
layout (location = 1) in vec2 aTexCoord;  // UVs
layout (location = 2) in int  aIndex;     // shape index

uniform int   uCount;
uniform vec2  uPositions[128];   // pixel-space top-left positions
uniform vec2  uSizes[128];       // pixel-space sizes
uniform float uRotations[128];
uniform vec4  uColors[128];
uniform int   uUseTexture[128];

uniform vec4 uTexRegionPx;
uniform vec2 uTextureSize;

uniform vec2  screenSize;        // window size in pixels

out vec2 vTexCoord;
out vec4 vColor;
out float vUseTexture;

vec2 pixelToNDC(vec2 p)
{
    // Convert from pixel coords (0..screen) → NDC (-1..+1)
    vec2 ndc = p / screenSize * 2.0 - 1.0;
    ndc.y *= -1.0; // flip Y so top-left is (0,0)
    return ndc;
}

void main()
{
    int i = aIndex;
    if (i >= uCount) {
        gl_Position = vec4(2.0, 2.0, 0.0, 1.0);
        vTexCoord = vec2(0.0);
        vColor = vec4(0.0);
        vUseTexture = 0.0;
        return;
    }

    vec2 pos = uPositions[i];    // top-left corner in pixels
    vec2 size = uSizes[i];
    float rot = uRotations[i];
    vec4 color = uColors[i];

    // Treat aPos as (0..1) local coords for top-left origin
    vec2 local = aPos * size;

    pos += size / 2.0;

    // Apply rotation about top-left if needed (optional center pivot below)
    float c = cos(rot);
    float s = sin(rot);
    vec2 pivot = vec2(0.5 * size.x, 0.5 * size.y); // rotate around center
    vec2 rotated = (local - pivot);
    rotated = vec2(rotated.x * c - rotated.y * s,
                   rotated.x * s + rotated.y * c) + pivot;

    // Add to top-left pixel position
    vec2 pixelPos = pos + rotated;

    gl_Position = vec4(pixelToNDC(pixelPos), 0.0, 1.0);

    vTexCoord = aTexCoord * uTexRegionPx.zw + uTexRegionPx.xy;

    //vTexCoord = (aTexCoord * uTexRegionPx.zw + uTexRegionPx.xy) / uTextureSize;
    vColor = color;
    vUseTexture = float(uUseTexture[i]);
}
