#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;

varying vec4 texCoord;

void main()
{
    
	vec4 pos = texture2D( positions, texCoord.st );
	vec4 vel = texture2D( velocities, texCoord.st );
    vec4 info = texture2D( information, texCoord.st );

    //position
	gl_FragData[0] = pos;
    //velocity
	gl_FragData[1] = vel;
    //information
	gl_FragData[2] = info;
}
