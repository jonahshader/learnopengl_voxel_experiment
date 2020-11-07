#version 430 core

in float fogMix_;
in vec2 texXY_;
flat in uint texture_;

uniform sampler2DArray sampler;
uniform vec3 skyColor;

out vec4 FragColor;

void main()
{
    vec4 col = texture(sampler, vec3(texXY_, texture_));

    FragColor = mix(col, vec4(skyColor, 1.0f), fogMix_);
}