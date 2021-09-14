#version 430 core

#define MAX_BRIGHTNESS (45.0f)
#define MIN_BRIGHTNESS (0.1f) // from 0 to 1

layout (location = 0) in ivec3 xyz;
layout (location = 1) in uint texture;
layout (location = 2) in ivec2 texCoord;
layout (location = 3) in uint b;
layout (location = 4) in uint normal;

out vec2 texCoord_;
flat out uint texture_;
flat out int normal_;
out float brightness_;
out float fogMix_;

uniform ivec3 chunkPos;
uniform mat4 viewProjection;
uniform vec3 camPos;
uniform uint chunkSize;
uniform float fogDistance;

uniform float xMinFog;
uniform float xMaxFog;
uniform float yMinFog;
uniform float yMaxFog;
uniform float zMinFog;
uniform float zMaxFog;


void main()
{
    vec3 posBeforeTransform = xyz + vec3(chunkPos) * chunkSize;
    gl_Position = viewProjection * vec4(posBeforeTransform, 1.0f);
    int normInt = int(normal);

    texCoord_ = texCoord;
    texture_ = texture;
    normal_ = normInt;
    brightness_ = (MIN_BRIGHTNESS + (float(b + 0.0f) / (MAX_BRIGHTNESS + 0.0f))) / (1.0f + MIN_BRIGHTNESS);
    float globalFogMix = pow(min(length(posBeforeTransform - camPos) / (fogDistance * 2), 1.0f), 2.0f); // the times 2 is temporary

    float xChunkLocalFogMix = mix(xMinFog, xMaxFog, float(xyz.x) / chunkSize);
    float yChunkLocalFogMix = mix(yMinFog, yMaxFog, float(xyz.y) / chunkSize);
    float zChunkLocalFogMix = mix(zMinFog, zMaxFog, float(xyz.z) / chunkSize);
    float chunkLocalFogMix = max(max(xChunkLocalFogMix, yChunkLocalFogMix), zChunkLocalFogMix);
//    fogMix_ = max(globalFogMix, chunkLocalFogMix);
//    fogMix_ = globalFogMix * .5f + chunkLocalFogMix * .5f;
    fogMix_ = chunkLocalFogMix;
}
