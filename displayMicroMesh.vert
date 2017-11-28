#version 410
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform sampler2D terrain;
uniform sampler2D waterPos;
uniform sampler2D waterHeight;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec2 Vertex;

out Attribs {
    vec3 pos;
    vec2 texuv;
} Out;

// takes a simple 2D vertex on the ground plane, offsets it along y by the land or water offset and projects it on screen
void main(void)
{
    Out.texuv = Vertex;
    /* TODO setup Linear sampler */
    vec4 pos = texture(waterPos, Vertex);
    if (pos.w <= 0.0) { /* No position data -> no water here */
        Out.pos = vec3(0);
        gl_Position = vec4(0,0,0,-1);
    } else {
        /* TODO setup Linear sampler */
        Out.pos = pos.xyz + texture(waterHeight, Vertex).rgb;
        gl_Position = matrProj * matrVisu * matrModel * vec4(Out.pos, 1);
    }
}

