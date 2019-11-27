#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

uniform sampler2D ourTexture;

out float intensity;

float sum(vec3 v) {
    return v[0] + v[1] + v[2];
}

void main()
{
    intensity = sum(texture(ourTexture, texCoord).xyz) * 0.05f;
    vec3 npos = position + normal * intensity;
    gl_Position = projection * view * model * vec4(npos, 1.0f);
}