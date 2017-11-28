#version 410
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform sampler2D terrain;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec2 Vertex;

out Attribs {
    vec3 pos;
} Out;

// takes a simple 2D vertex on the ground plane, offsets it along y by the land or water offset and projects it on screen
void main(void)
{
    vec2 uv = 0.5 * (Vertex + 1);
    float h = 0.001 * (-3.5 + 80 * texture(terrain, uv).r);
    vec3 pos = SCENE_EXTENT * 0.5 * vec3(Vertex.x, h, Vertex.y);
    Out.pos = pos;
    gl_Position = matrProj * matrVisu * matrModel * vec4(pos, 1);
}

