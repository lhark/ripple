#version 410
#define PI 3.14159265359

out vec4 Color;

in Attribs {
    vec4 Pos;
    vec4 Att1;
    vec4 Att2;
} In;

// rasterize wave packet quad
// wave packet data:
// position vector: x,y = [-1..1], position in envelope
// attribute vector: x=amplitude, y=wavelength, z=time phase, w=envelope size
// FALSE /!\ attribute2 vector: (x,y)=position of bending point, z=central distance to ref point, 0
void main(void)
{
    /* If centerDiff == 0, the wave is straight */
    float centerDiff = length(In.Pos.zw - vec2(0, In.Att2.x)) - abs(In.Pos.w - In.Att2.x);
    float phase = -In.Att1.z + (In.Pos.w + centerDiff) * 2 * PI / In.Att1.y;
    /* ROLL CREDITS ! */
    vec3 ripple = (1 + cos(In.Pos.x * PI)) * (1 + cos(In.Pos.y * PI)) * In.Att1.x *
                  vec3(0, cos(phase), 0);
    Color = vec4(ripple, 1);
}
