#version 330 core

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

layout (location=0) in vec3 pos;
layout (location=1) in vec2 texCoord;
layout (location=2) in vec3 norm;

out vec4 vertex;

void main()
{
	vec4 tpos = model * vec4(pos, 1.0f);
	vertex = projection * view * tpos;
	gl_Position = vertex;
}