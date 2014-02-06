#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform sampler2D noiseTex;
uniform float dt;
uniform vec3 constraints;
uniform bool reset;
uniform bool takeFormation;
uniform float formationStep;
uniform int motion;

varying vec4 texCoord;

const float eps = 0.001;

void main()
{
    
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
    
	float decay = texture2D( information, texCoord.st ).r;
	float maxAge = texture2D( information, texCoord.st ).g;
    
    
    if (takeFormation) {
        vel = vec3(0.0,0.0,0.0);
        vec3 targetPos = texture2D( oPositions, texCoord.st ).rgb;
        
        pos = mix(pos,targetPos,formationStep);
    }
    else
    {
        vec3 noise = texture2D( noiseTex, pos.xy ).rgb;
        //vec3 force = vec3(noise - pos.xy,0.0);
        vec3 force = vec3(noise.x,noise.y,noise.z);
        float fMag = length(force);
        vec3 a = invMass * force;// * pow(fMag,0.9);
        vel += dt * a;
        
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
        pos.z += vel.z * dt;
    }
	
    
	
    // reincarnation
	if( reset )
    {
        vec3 origVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 origPos = texture2D(oPositions, texCoord.st).rgb;
        pos = origPos;
        vel = origVel;
    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, invMass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, age);
    //age information
	//gl_FragData[2] = vec4(age, maxAge, 0.0, 1.0);
}


