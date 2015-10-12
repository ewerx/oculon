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
uniform bool startAnim;
uniform float formationStep;
//uniform int motion;
uniform float containmentSize;


uniform sampler2D nodePosTex;
uniform float nodeBufSize;
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

    // animate to formation
    if (startAnim)
    {
        startPos = pos;
    }
    
    float numNodes = nodeBufSize * nodeBufSize;
    vec2 uv;
    uv.x = float( mod( nodeIndex, nodeBufSize ) ) / nodeBufSize;
    uv.y = float( floor( nodeIndex / nodeBufSize ) ) / nodeBufSize;
    
    vec3 targetPos = texture2D( nodePosTex, uv ).rgb;
    
    if (formationStep < 1.0)
    {
        //vel = vec3(0.0,0.0,0.0);
        pos = mix(startPos, targetPos, formationStep);
    }
    else
    {
        pos = targetPos;
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


