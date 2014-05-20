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
uniform float containmentSize;

uniform sampler2D audioData;
uniform float gain;

varying vec4 texCoord;

const float eps = 0.001;

void main()
{
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
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
    else if (doSim) // simulate motion
    {
        if (motion == 0)
        {
            // static, no sim
        }
        else
        {
            if (motion == 1) // force by noise
            {
                vec3 noise = texture2D( noiseTex, pos.xy ).rgb;
                vec3 force = noise - pos;
                //vec3 force = vec3(noise.x,noise.y,noise.z);
                //float fMag = length(force);
                vec3 a = invMass * force;// * pow(fMag,0.9);
                vel = dt * a;
                pos += vel * dt;
            }
            else if (motion == 2) // gravition -- single source at center
            {
                vec3 a = invMass * -pos/(dist*dist);
                vel += dt * a;
                pos += vel * dt;
            }
            else if (motion == 3) // audio-reactive positions (wave)
            {
                pos = texture2D(oPositions, texCoord.st).rgb + texture2D( audioData, vec2(texCoord.s * texCoord.t,1.0) ).x * gain * 10.0;
            }
            else if (motion == 4) // audio-reactive positions (fft)
            {
                pos = texture2D(oPositions, texCoord.st).rgb + vel * texture2D( audioData, vec2(texCoord.s * texCoord.t,0.0) ).x * gain * 10.0;
            }
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


