#version 430 core

out vec4 FragColor;

in vec2 texCoord_;
flat in uint texture_;
flat in int normal_;
in float brightness_;

uniform sampler2DArray sampler;

void main()
{
    // TODO: use normal to manipulate brightness somehow idk
    // probably take in a lookup table as a uniform that converts normal to brightness
    // this would just be for sunlight direction, idk how to do reflectance stuff yet
    FragColor = texture(sampler, vec3(texCoord_, texture_));
//    FragColor = texture(sampler, vec3(0.5f, 0.5f, 3.0f));
//    FragColor = vec4(texCoord_, float(texture_), 1.0f);
//    FragColor = vec4(1.f, 0.f, 0.f, 1.f);
//    FragColor.rgb = texCoord_;
}