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
uniform bool startAnim;
uniform float formationStep;
uniform int motion;
uniform float containmentRadius;

uniform sampler2D audioData;
uniform float gain;

varying vec4 texCoord;

void main()
{
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
    // animate to formation
    if (startAnim)
    {
        startPos = pos;
    }
    
    if (formationStep < 1.0)
    {
        //vel = vec3(0.0,0.0,0.0);
        vec3 targetPos = texture2D( oPositions, texCoord.st ).rgb;
        pos = mix(startPos,targetPos,formationStep);
    }
    else
    {
        if (motion == 0)
        {
            // static, no sim
        }
        else
        {
            float dist = length(pos);
            
            if (motion == 1) // bounce
            {
                // containment
                if (containmentRadius > 5.0 && dist > containmentRadius || dist > 1000.0)
                {
                    vec3 norm = normalize(pos);
                    pos = norm * containmentRadius;
                    
                    vec3 u = dot(vel,-norm) * -norm;
                    vec3 w = vel - u;
                    // v' = f*w - r*u (friction / restitution)
                    vel = w - u;
                }
            }
            else if (motion == 2) // force by noise
            {
                vec3 noise = texture2D( noiseTex, pos.xy ).rgb;
                vec3 force = noise - pos;
                //vec3 force = vec3(noise.x,noise.y,noise.z);
                //float fMag = length(force);
                vec3 a = invMass * force;// * pow(fMag,0.9);
                vel = dt * a;
                pos += vel * dt;
                
                // containment
                if (containmentRadius > 5.0 && dist > containmentRadius || dist > 1000.0)
                {
                    vec3 norm = normalize(pos);
                    pos = norm * containmentRadius;
                    
                    vec3 u = dot(vel,-norm) * -norm;
                    vec3 w = vel - u;
                    // v' = f*w - r*u (friction / restitution)
                    vel = w - u;
                }
            }
//            else if (motion == 3) // audio-reactive positions (wave)
//            {
//                pos = texture2D(oPositions, texCoord.st).rgb + texture2D( audioData, vec2(texCoord.s * texCoord.t,1.0) ).x * gain * 10.0;
//            }
//            else if (motion == 4) // audio-reactive positions (fft)
//            {
//                pos = texture2D(oPositions, texCoord.st).rgb + vel * texture2D( audioData, vec2(texCoord.s * texCoord.t,0.0) ).x * gain * 10.0;
//            }
            
        }
    }
	
    // reincarnation
	if( reset )
    {
        pos = texture2D(oPositions, texCoord.st).rgb;
        vel = texture2D(oVelocities, texCoord.st).rgb;
    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, invMass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, age);
    //age information
	gl_FragData[2] = vec4(startPos, decay);
}


