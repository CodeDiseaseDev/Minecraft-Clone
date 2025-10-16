#include "SkinUVRegistry.h"

constexpr float TEX_W = 64.0f;
constexpr float TEX_H = 32.0f;

FaceUV SkinUVRegistry::makeFace(float x, float y, float w, float h) {
    float u0 = x;
    float v0 = y;
    float u1 = (x + w);
    float v1 = (y + h);

    // OpenGL’s UV origin is bottom-left, Minecraft’s is top-left → flip vertically
    v0 = 1.0f - v0;
    v1 = 1.0f - v1;

    return {
        {u0 + default_skin_uv_x, v1 + default_skin_uv_y}, // bottom-left
        {u1 + default_skin_uv_x, v1 + default_skin_uv_y}, // bottom-right
        {u1 + default_skin_uv_x, v0 + default_skin_uv_y}, // top-right
        {u0 + default_skin_uv_x, v0 + default_skin_uv_y}  // top-left
    };
}

// --- HEAD (8x8 cube at (8,8)) ---
FaceUV SkinUVRegistry::getHeadUV(SkinFaceIndex face) {
    switch (face) {
        case SkinFaceIndex::POS_X: return makeFace(16, 8, 8, 8); // left
        case SkinFaceIndex::NEG_X: return makeFace(0, 8, 8, 8);  // right
        case SkinFaceIndex::POS_Y: return makeFace(8, 0, 8, 8);  // top
        case SkinFaceIndex::NEG_Y: return makeFace(16, 0, 8, 8); // bottom
        case SkinFaceIndex::POS_Z: return makeFace(8, 8, 8, 8);  // front
        case SkinFaceIndex::NEG_Z: return makeFace(24, 8, 8, 8); // back
    }
    return makeFace(0, 0, 0, 0);
}

// --- TORSO (12x8x4, main body at (20,16)) ---
FaceUV SkinUVRegistry::getTorsoUV(SkinFaceIndex face) {
    switch (face) {
        case SkinFaceIndex::POS_X: return makeFace(28, 20, 4, 12); // left
        case SkinFaceIndex::NEG_X: return makeFace(16, 20, 4, 12); // right
        case SkinFaceIndex::POS_Y: return makeFace(20, 16, 8, 4);  // top
        case SkinFaceIndex::NEG_Y: return makeFace(28, 16, 8, 4);  // bottom
        case SkinFaceIndex::POS_Z: return makeFace(20, 20, 8, 12); // front
        case SkinFaceIndex::NEG_Z: return makeFace(32, 20, 8, 12); // back
    }
    return makeFace(0, 0, 0, 0);
}

// --- ARM (12x4x4, at (44,16)) ---
FaceUV SkinUVRegistry::getArmUV(SkinFaceIndex face) {
    switch (face) {
        case SkinFaceIndex::POS_X: return makeFace(48, 20, 4, 12);
        case SkinFaceIndex::NEG_X: return makeFace(40, 20, 4, 12);
        case SkinFaceIndex::POS_Y: return makeFace(44, 16, 4, 4);
        case SkinFaceIndex::NEG_Y: return makeFace(48, 16, 4, 4);
        case SkinFaceIndex::POS_Z: return makeFace(44, 20, 4, 12);
        case SkinFaceIndex::NEG_Z: return makeFace(52, 20, 4, 12);
    }
    return makeFace(0, 0, 0, 0);
}

// --- LEG (12x4x4, at (4,16)) ---
FaceUV SkinUVRegistry::getLegUV(SkinFaceIndex face) {
    switch (face) {
        case SkinFaceIndex::POS_X: return makeFace(8, 20, 4, 12);
        case SkinFaceIndex::NEG_X: return makeFace(0, 20, 4, 12);
        case SkinFaceIndex::POS_Y: return makeFace(4, 16, 4, 4);
        case SkinFaceIndex::NEG_Y: return makeFace(8, 16, 4, 4);
        case SkinFaceIndex::POS_Z: return makeFace(4, 20, 4, 12);
        case SkinFaceIndex::NEG_Z: return makeFace(12, 20, 4, 12);
    }
    return makeFace(0, 0, 0, 0);
}
