#version 430 core

layout (location = 0) in vec3 xyz;
layout (location = 1) in vec2 texXY;

layout (location = 2) in vec3 offset;
layout (location = 3) in vec2 texOffset;
layout (location = 4) in vec2 texSize;
layout (location = 5) in vec2 bbSize;

//uniform ivec3 chunkPos;
uniform mat4 viewProjection;
uniform vec3 camPos;
uniform uint chunkSize;
uniform float fogDistance;

out float fogMix_;
out vec2 texCoord_;

void main()
{
    vec2 transformedTexCoord = (texXY * texSize) + texOffset;
    vec3 posScaled = xyz;
    posScaled.xz *= bbSize.x;
    posScaled.y *= bbSize.y;
    vec3 posBeforeTransform = posScaled + offset;
    gl_Position = viewProjection * vec4(posBeforeTransform, 1.0f);

    fogMix_ = min(length(posBeforeTransform - camPos) / fogDistance, 1.0f);
    texCoord_ = transformedTexCoord;
}