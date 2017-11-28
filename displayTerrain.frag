#version 410
#define PI 3.14159265359

out vec4 Color;

in Attribs {
    vec3 pos;
} In;

void main(void)
{
    if (In.pos.y < - 0.1)
        discard; /* Fragment underwater */
    Color.rgb = (0.25 + 0.75 * In.pos.y) * vec3(0.75);
    Color.a = 1.0;
}

