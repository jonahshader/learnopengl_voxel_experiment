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


void main()
{
    vec3 posBeforeTransform = xyz + vec3(chunkPos) * chunkSize;
    gl_Position = viewProjection * vec4(posBeforeTransform, 1.0f);
    int normInt = int(normal);

    texCoord_ = texCoord;
    texture_ = texture;
    normal_ = normInt;
    brightness_ = (MIN_BRIGHTNESS + (float(b + 0.0f) / (MAX_BRIGHTNESS + 0.0f))) / (1.0f + MIN_BRIGHTNESS);
//    fogMix_ = pow(min(length(posBeforeTransform - camPos) / fogDistance, 1.0f), 2.0f);
    fogMix_ = min(length(posBeforeTransform - camPos) / fogDistance, 1.0f);
}
