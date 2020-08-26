#version 430 core

#define CHUNK_SIZE (32)
#define MAX_BRIGHTNESS (15.f)

layout (location = 0) in vec3 xyz;
layout (location = 1) in vec2 tcoord;
layout (location = 2) in float n;

// instanced
layout (location = 3) in ivec3 offset;
layout (location = 4) in ivec3 dims;
layout (location = 5) in uint texs[6];
layout (location = 11) in uint b;


out vec2 texCoord_;
flat out uint texture_;
flat out int normal_;
out float brightness_;

uniform ivec3 chunkPos;
uniform mat4 viewProjection;

void main()
{
    gl_Position = viewProjection * vec4(xyz * dims + vec3(offset) + vec3(chunkPos) * CHUNK_SIZE, 1.0f);
    int normInt = int(n);



    switch (normInt) {
        case 0:
            texCoord_ = tcoord * dims.xy;
        break;
        case 1:
            texCoord_ = tcoord * dims.zy;
        break;
        case 2:
            texCoord_ = tcoord * dims.xy;
        break;
        case 3:
            texCoord_ = tcoord * dims.zy;
        break;
        case 4:
            texCoord_ = tcoord * dims.xz;
        break;
        case 5:
            texCoord_ = tcoord * dims.xz;
        break;
    }


    texture_ = texs[normInt];
    normal_ = normInt;
    brightness_ = float(b) / MAX_BRIGHTNESS;
}
