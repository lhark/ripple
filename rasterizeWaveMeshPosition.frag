#version 410

in Attribs {
    vec3 pos;
} In;

out vec4 Color;

void main(void)
{
    Color = vec4(In.pos, 1.);
}

