#version 330 core

in float intensity;

out vec4 color;

void main()
{
    color = vec4(intensity);
    color[3] = 1.0f;
}