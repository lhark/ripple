#version 410
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Pos;
layout(location=4) in vec4 Att1;
layout(location=8) in vec4 Att2;

out Attribs {
    vec4 Pos;
    vec4 Att1;
    vec4 Att2;
} Out;

void main(void)
{
    Out.Pos = Pos;
    Out.Att1 = Att1;
    Out.Att2 = Att2;
    gl_PointSize = 5;
}
