#version 430 core

#define CHUNK_SIZE (32)
#define MAX_BRIGHTNESS (15.f)

layout (location = 0) in uvec3 xyz;

layout (location = 1) in uvec2 tcoord;
layout (location = 2) in uint n;
layout (location = 3) in uint tex;
layout (location = 4) in uint b;

//layout (location = 0) in uint x;
//layout (location = 1) in uint y;
//layout (location = 2) in uint z;
//
//layout (location = 3) in uint tx;
//layout (location = 4) in uint ty;
//layout (location = 5) in uint n;
//layout (location = 6) in uint tex;
//layout (location = 7) in uint b;

out vec2 texCoord_;
flat out uint texture_;
//flat out int normal_;
out float brightness_;

uniform ivec3 chunkPos;
uniform mat4 viewProjection;

void main()
{
    gl_Position = viewProjection * vec4(vec3(xyz) + vec3(chunkPos) * CHUNK_SIZE, 1.0f);
//    gl_Position = viewProjection * vec4(xyz, 1.0f);
//    gl_Position = viewProjection * vec4(float(x), float(y), float(z), 1.0f);
    texCoord_ = tcoord;
    texture_ = tex;
//    normal_ = n;
    brightness_ = float(b) / MAX_BRIGHTNESS;
}
