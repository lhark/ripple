#version 410
#define SCENE_EXTENT 100.0f

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

/////////////////////////////////////////////////////////////////

in Attribs {
    vec3 pos;
    vec2 texuv;
} In[];

out Attribs {
    vec3 pos;
    vec2 texuv;
} Out;

void main(void)
{
    if (gl_in[0].gl_Position.w<0.01 || gl_in[1].gl_Position.w<0.01 || gl_in[2].gl_Position.w<0.01)
        return;
    for (int i = 0; i < 3; i++) {
        Out.pos = In[i].pos;
        Out.texuv = In[i].texuv;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
}
