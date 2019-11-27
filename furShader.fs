#version 330 core

in vec2 TexCoords;
in float alpha_channel;

in vec3 normal;
in vec3 binormal;
in vec3 tangent;

uniform sampler2D furTexture;

float sum(vec3 c) {
    return c[0] + c[1] + c[2];
}

void main() {
    float alpha = alpha_channel;
    gl_FragColor = texture(furTexture, TexCoords);
 
    gl_FragColor[3] *= alpha;
}