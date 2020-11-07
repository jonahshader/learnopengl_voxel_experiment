#version 430 core

out vec4 FragColor;

in vec2 texCoord_;
flat in uint texture_;
flat in int normal_;
in float brightness_;
in float fogMix_;

uniform sampler2DArray sampler;
uniform vec3 skyColor;

void main()
{
    // TODO: use normal to manipulate brightness somehow idk
    // probably take in a lookup table as a uniform that converts normal to brightness
    // this would just be for sunlight direction, idk how to do reflectance stuff yet
    vec4 col = texture(sampler, vec3(texCoord_, texture_));
    col.rgb *= brightness_;
    FragColor = mix(col, vec4(skyColor, 1.0f), fogMix_);
}