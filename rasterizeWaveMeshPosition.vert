#version 410
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

/////////////////////////////////////////////////////////////////

in vec2 Vertex;
in vec2 TexCoord;

out Attribs {
    vec3 pos;
} Out;

void main(void)
{
    Out.pos = 0.5 * SCENE_EXTENT * vec3(Vertex.x, 0, Vertex.y);
    gl_Position = matrProj * matrVisu * matrModel * vec4(Out.pos, 1.);
}

