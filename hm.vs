#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

uniform sampler2D ourTexture;

out vec2 TexCoord;

float sum(vec3 v) {
    return v[0] + v[1] + v[2];
}

void main()
{
    float intensity = sum(texture(ourTexture, texCoord).rrr) * 0.05;
    vec3 npos = position + vec3(0, 0, 1) * intensity * 0.2;
    gl_Position = projection * view * model * vec4(npos, 1.0f);
	TexCoord = texCoord;
}