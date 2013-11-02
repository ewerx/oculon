#version 120
uniform sampler2D pointSpriteTex;

varying vec4 color;

void main(void)
{
    vec4 tex = texture2D(pointSpriteTex, gl_PointCoord);
    gl_FragColor = tex * vec4(2.0*abs(color.xyz)+color.w, 1.0);
}