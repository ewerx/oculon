#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform sampler2D noiseTex;
uniform float dt;
//uniform vec3 constraints;
uniform bool reset;
//uniform bool startAnim;
//uniform float formationStep;
//uniform int motion;
uniform float containmentSize;


//uniform sampler2D nodePosTex;
//uniform float nodeBufSize;
//
//uniform sampler2D audioData;
//uniform float gain;

varying vec4 texCoord;

void main()
{
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float nodeIndex = texture2D( information, texCoord.st ).a;
    
    bool doSim = true; // to skip simulation when forcing containment...
    float dist = length(pos);
    // containment
    {
        if (containmentSize > 1.0 && dist > containmentSize)
        {
            vec3 norm = normalize(pos);
            pos = norm * containmentSize;
            vel *= -0.999;
            doSim = false;
        }
//        if(pos.x > containmentSize || pos.x < -containmentSize || pos.y > containmentSize || pos.y < -containmentSize || pos.z > containmentSize || pos.z < -containmentSize) {
//            vel *= -0.9;
//        }
    }
    
    if (doSim) // simulate motion
    {
        vec3 noise = texture2D( noiseTex, pos.xy ).rgb;
        vec3 force = noise;// - pos;
        //vec3 force = vec3(noise.x,noise.y,noise.z);
        //float fMag = length(force);
        vec3 a = invMass * force;// * pow(fMag,0.9);
        vel = dt * a;
        pos += vel * dt;
    }
	
    // reincarnation
	if( reset )
    {
        pos = texture2D(oPositions, texCoord.st).rgb;
        vel = texture2D(oVelocities, texCoord.st).rgb;
    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, invMass);
    //velocity + nodeIndex
	gl_FragData[1] = vec4(vel, age);
    //age information
	gl_FragData[2] = vec4(startPos, nodeIndex);
}


