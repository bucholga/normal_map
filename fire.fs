#version 330 core

uniform sampler2D sprite;
uniform sampler2D colorMap;

in float phase;
out vec4 color;

void main ()
{
    vec4 c1 = texture ( sprite, gl_PointCoord );
    vec4 c2 = texture ( colorMap, vec2 ( 1.0 - phase, 1.0 ) );
    color = (1.0 - phase) * vec4 ( c1.rgb * c2.rgb, 1.0 );
}