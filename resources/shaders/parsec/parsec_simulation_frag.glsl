#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;

varying vec4 texCoord;

const float eps = 0.001;

void main()
{
    
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
	
    //position + mass
	gl_FragData[0] = vec4(pos, invMass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, age);
    //age information
	//gl_FragData[2] = vec4(age, maxAge, 0.0, 1.0);
}
