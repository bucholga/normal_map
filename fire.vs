#version 330 core
layout ( location = 0 ) in vec4 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out float phase;
const vec3 velocity = vec3 ( 0.0, 0.0, 2.0 );
const float period = 4.0;
void main()
{
   vec3 pos = aPos.xyz;
   phase = fract ( aPos.w + time / period );
   gl_Position = projection* view * vec4 ( pos + phase * velocity, 1.0 );
   gl_PointSize = 8.0 + 20.0 * phase;
}