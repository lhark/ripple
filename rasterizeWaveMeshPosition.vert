#version 410
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec2 Vertex;
layout(location=8) in vec2 TexCoord;

out Attribs {
    vec3 pos;
} Out;

void main(void)
{
    Out.pos = 0.5 * SCENE_EXTENT * vec3(Vertex.x, Vertex.y, 0);
    gl_Position = matrProj * matrVisu * vec4(Out.pos, 1.);
}

