//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
varying vec4 texCoord;

void main(void)
{
    //Render to positions texture
    gl_FragData[0] = texture2D( positions, texCoord.st);
    //Render to velocities texture
    gl_FragData[1] = texture2D( velocities, texCoord.st);
}

