#version 120
uniform sampler2D pointSpriteTex;
uniform vec4 colorScale;

varying vec4 color;

void main(void)
{
    vec4 tex = texture2D(pointSpriteTex, gl_PointCoord);
    tex *= colorScale;
    tex.a *= (colorScale.a + 10.0*color.a);
    gl_FragColor = tex;
}