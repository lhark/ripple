#version 410
#define SCENE_EXTENT 100.0f

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

/////////////////////////////////////////////////////////////////

in Attribs {
    vec4 Pos;
    vec4 Att1;
    vec4 Att2;
} In[];

out Attribs {
    vec4 Pos;
    vec4 Att1;
    vec4 Att2;
} Out;

void main(void)
{
    if (In[0].Pos.x < -9000)
        return;
    Out.Att1 = In[0].Att1;
    Out.Att2 = In[0].Att2;
    float dThickness = In[0].Att1.w;
    float wThickness = In[0].Att1.w;
    vec3 center = vec3(In[0].Pos.x, 0, In[0].Pos.y);
    vec3 wavedir = vec3(In[0].Pos.z, 0, In[0].Pos.w);
    vec3 wavecrest = vec3(wavedir.z, 0, -wavedir.x); // wave crest, orthogonal to wave direction
    vec4 p1 = vec4(center - wThickness * wavecrest, 1);
    vec4 p2 = vec4(center - wThickness * wavecrest - dThickness * wavedir, 1);
    vec4 p3 = vec4(center + wThickness * wavecrest, 1);
    vec4 p4 = vec4(center + wThickness * wavecrest - dThickness * wavedir, 1);
    Out.Pos = vec4(-1, 1, -In[0].Att1.w, 0);
    gl_Position = matrProj * matrVisu * matrModel * p1;
    EmitVertex();
    Out.Pos = vec4(-1, -1, -In[0].Att1.w, -In[0].Att1.w);
    gl_Position = matrProj * matrVisu * matrModel * p2;
    EmitVertex();
    Out.Pos = vec4(1, 1, -In[0].Att1.w, 0);
    gl_Position = matrProj * matrVisu * matrModel * p3;
    EmitVertex();
    Out.Pos = vec4(1, -1, In[0].Att1.w, -In[0].Att1.w);
    gl_Position = matrProj * matrVisu * matrModel * p4;
    EmitVertex();
}

