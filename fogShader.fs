#version 330 core

in vec4 vertex;
uniform vec4 lightColor;

vec4 diffuse_color(vec4 lightColor, vec4 surfaceColor) {
    float diffuseFactor = clamp(dot(lightColor, surfaceColor),0.0,1.0);
    return lightColor * surfaceColor * diffuseFactor;
}

void main()
{
    const float blur = 0.7f;
    const float fh0 = 0.1f;
    const float e = 2.718281828;
    float fh = (vertex.y / vertex.w);
    float fogFactor;

    if (fh < fh0) {
        fogFactor = 1.0f;
    } else {
        fogFactor = blur * (fh - fh0);
        fogFactor *= fogFactor;
        fogFactor = clamp(pow(e, -blur * fogFactor), 0.0f, 1.0f);
    }
    const vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    vec4 calc = diffuse_color(lightColor, vec4(1.0f));
    vec4 mixFog = mix(calc, fogColor, fogFactor);
    gl_FragColor = mixFog;
}