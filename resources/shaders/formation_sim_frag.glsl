#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;

uniform bool reset;

uniform bool startAnim;
uniform float formationStep;

varying vec4 texCoord;

void main()
{
	vec4 vPos = texture2D( positions, texCoord.st );
	vec4 vVel = texture2D( velocities, texCoord.st );
    vec4 vInfo = texture2D( information, texCoord.st );
    
    vec3 targetPos = texture2D(oPositions, texCoord.st).rgb;
    vec3 targetVel = texture2D(oVelocities, texCoord.st).rgb;
    
    vec3 pos = vPos.xyz;
    vec3 vel = vVel.xyz;
    vec3 startPos = vInfo.xyz;
    
    // animate to formation
    if (startAnim)
    {
        startPos = pos;
        vel = targetVel;
    }
    
    // reincarnation
    if( reset )
    {
        pos = targetPos;
        vel = targetVel;
    }
    else if (formationStep < 1.0)
    {
        //vel = vec3(0.0,0.0,0.0);
        pos = mix(startPos, targetPos, formationStep);
    }
    
    //position
	gl_FragData[0] = vec4(pos, vPos.w);
    //velocity
	gl_FragData[1] = vec4(vel, vVel.w);
    //information
	gl_FragData[2] = vec4(startPos, vInfo.w);
}
