#version 430 core

in float fogMix_;
in vec2 texCoord_;

uniform sampler2D sampler;
uniform vec3 skyColor;

out vec4 FragColor;

void main()
{
    vec4 col = texture(sampler, texCoord_);

    if (col.a < 0.1)
        discard;

    FragColor = mix(col, vec4(skyColor, 1.0f), fogMix_);
//    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}