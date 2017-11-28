#version 410
#define PI 3.14159265359
#define SCENE_EXTENT 100.0f

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform sampler2D terrain;
uniform sampler2D waterPos;
uniform sampler2D waterHeight;

out vec4 Color;

in Attribs {
    vec3 pos;
    vec2 texuv;
} In;

void main(void)
{
	// the derivative of the water displacement texture gives us the water surface normal
    /*TODO Linear sampler */
    vec3 pos = texture(waterPos, In.texuv).rgb + texture(waterHeight, In.texuv).rgb;
    vec3 N = cross(dFdx(pos), -dFdy(pos));
    if (dot(N, N) <= 0.0)
        N = vec3(0, -1, 0);
    else
        N = normalize(N);
    vec3 V = normalize(In.pos - (matrVisu * matrModel)[3].xyz); /* TODO check view vector */
    vec3 R = V - (2 * dot(V, N))*N; /* Reflection */
    /* Diffuse + reflective lighting */
    vec3 color = vec3(0.5, 0.6, 0.8);
    float fac = 1.0 - (1.0 - abs(N.y) + abs(R.y)) * (1.0 - abs(N.y) + abs(R.y));
    Color.rgb = fac * fac * color;
    /* Add specular glares */
    vec3 glareDir1 = normalize(vec3(-1, -0.75,  1));
    vec3 glareDir2 = normalize(vec3( 1, -0.75, -1));
    vec3 glareDir3 = normalize(vec3( 1, -0.75,  1));
    vec3 glareDir4 = normalize(vec3(-1, -0.75, -1));
    vec3 glareDir5 = normalize(vec3( 0,    -1,  0));
    Color.rgb += 100*pow(max(dot(-R, glareDir5), max(dot(-R, glareDir4), max(dot(-R, glareDir3), max(dot(-R, glareDir2), max(dot(-R, glareDir1), 0.0))))), 5000);
    /* Grid overlay */
    float floorFactor = 1.0;
    float sth = 0.06;
    float posfac = 1.2 * 80.0 / SCENE_EXTENT;
    if (fract(posfac * pos.x) < sth)
        floorFactor = 0.5 - 0.5 * cos(-PI + 2.0 * PI * fract(posfac * pos.x) / sth);
    if (fract(posfac * pos.z) < sth)
        floorFactor = min(floorFactor, 0.5 - 0.5 * cos(-PI + 2.0 * PI * fract(posfac * pos.z) / sth));
    Color.rgb *= (0.75 + 0.25 * floorFactor);
    /* TODO Linear sampling */
    float waterDepth = 1.0 + 0.9 * pow(texture(terrain, In.pos.xz / SCENE_EXTENT + vec2(0.5)).z, 4);
    Color.rgb *= waterDepth;
    Color.a = 1.0;
}
