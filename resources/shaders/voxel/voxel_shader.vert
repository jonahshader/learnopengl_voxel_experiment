#version 430 core

#define MAX_BRIGHTNESS (225.f)

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
out float fogMix_;

uniform ivec3 chunkPos;
uniform mat4 viewProjection;
uniform vec3 camPos;
uniform uint chunkSize;


void main()
{
    vec3 posBeforeTransform = xyz * dims + vec3(offset) + vec3(chunkPos) * chunkSize;
    gl_Position = viewProjection * vec4(posBeforeTransform, 1.0f);
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
    brightness_ = (float(b + 0.0f) / (MAX_BRIGHTNESS + 0.0f));;
    fogMix_ = min(length(posBeforeTransform - camPos) / 350.0f, 1.0f);
//    fogMix_ = length(posBeforeTransform - camPos);
//    fogMix_ = exp(-pow(fogMix_, 1.5f));
}
