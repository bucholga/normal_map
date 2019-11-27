#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform float mirror;

void main()
{    
    FragColor = texture(texture1, TexCoords);
    if (mirror > 0) FragColor = FragColor*0.5;
    FragColor[3] = 1.0;
}