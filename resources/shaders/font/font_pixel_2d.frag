#version 430 core
out vec4 FragColor;

in vec2 texCoord_;
in vec3 color_;

uniform sampler2D fontTexture;

void main()
{
    vec4 col = texture(fontTexture, texCoord_);
    if (col.r < 0.5)
        discard;

//    FragColor = texture(fontTexture, texCoord_) * vec4(color_, 1.0);
    FragColor = vec4(color_, 1.0);
}