#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform float alpha;
out float alpha_channel;

out vec3 normal;
out vec3 binormal;
out vec3 tangent;

mat3 rotate_y(float angle) {
	float PI = 3.14159265;
	angle = angle * PI / 180.0f;
	return mat3(
		cos(angle), 0, sin(angle),
		         0, 1,          0,
		-sin(angle), 0, cos(angle));
}

void main() {
	vec4 npos = view * model * vec4(aPos, 1.0f);
	gl_Position = projection * npos;
	TexCoords = vec2(aTexCoords.x, aTexCoords.y);
	normal = normalize(aPos);
	tangent = rotate_y(90.0f) * normal;
	binormal = cross(tangent, normal);
	alpha_channel = alpha;
}