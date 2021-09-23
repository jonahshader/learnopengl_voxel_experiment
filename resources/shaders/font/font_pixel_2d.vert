#version 430 core

//layout (location = 0) in vec2 xy;
//layout (location = 1) in vec2 texXY;
//layout (location = 2) in vec2 texSize;
//layout (location = 3) in vec2 renderSize;
//layout (location = 4) in vec3 color;

layout (location = 0) in vec2 xy;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

uniform mat4 viewProjection;

out vec2 texCoord_;
out vec3 color_;

void main()
{
    gl_Position = vec4(xy, 0.0, 1.0);
    color_ = color;
    texCoord_ = texCoord;
}