#version 120
uniform sampler2D pointSpriteTex;
uniform vec4 colorScale;
uniform bool audioReactive;

varying vec4 color;

void main(void)
{
    vec4 tex = texture2D(pointSpriteTex, gl_PointCoord);
    tex *= colorScale;
    if (audioReactive)
    {
        tex.a *= (tex.a + 10.0*color.a);
    }
    gl_FragColor = tex;
}