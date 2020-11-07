#version 430 core

layout (location = 0) in vec3 xyz;
layout (location = 1) in vec2 texXY;
layout (location = 2) in vec3 offset;
layout (location = 3) in uint texture;

uniform ivec3 chunkPos;
uniform mat4 viewProjection;
uniform vec3 camPos;
uniform uint chunkSize;
uniform float fogDistance;

out float foxMix_;
out vec2 texXY_;
flat out uint texture_;

void main()
{
    vec3 posBeforeTransform = xyz + offset + vec3(chunkPos) * chunkSize;
    gl_Position = viewProjection * vec4(posBeforeTransform, 1.0f);

    fogMix_ = min(length(posBeforeTransform - camPos) / fogDistance, 1.0f);
    texXY_ = texXY;
    texture_ = texture;
}