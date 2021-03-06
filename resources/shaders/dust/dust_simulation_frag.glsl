#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform sampler2D noiseTex;
uniform float dt;
uniform float decayRate;
uniform vec3 constraints;
uniform bool reset;
uniform bool startAnim;
uniform float formationStep;
uniform int motion;

varying vec4 texCoord;

void main()
{
    
	vec3 pos = texture2D( positions, texCoord.st ).rgb;
	float invMass = texture2D( positions, texCoord.st ).a;

	vec3 vel = texture2D( velocities, texCoord.st ).rgb;
	float age = texture2D( velocities, texCoord.st ).a;
    
	float decay = texture2D( information, texCoord.st ).r;
	float maxAge = texture2D( information, texCoord.st ).g;
    vec3 startPos = vec3( texture2D( information, texCoord.st ).ba, 0.0 );
    
    age += dt * decay * decayRate;
    
    // animate to formation
    if (startAnim)
    {
        startPos = pos;
    }
    
    if (formationStep < 1.0)
    {
        // take formation
        vel = vec3(0.0,0.0,0.0);
        vec3 targetPos = vec3( texture2D( oPositions, texCoord.st ).rg, 0.0 );
        
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
            if (motion == 1) // force by noise
            {
                // noise motion
                vec2 noise = texture2D( noiseTex, pos.xy ).rg;
                //vec3 force = vec3(noise - pos.xy,0.0);
                vec3 force = vec3(noise.x,noise.y,0.0);
                float fMag = length(force);
                vec3 a = invMass * force;// * pow(fMag,0.9);
                vel += dt * a;
            }
            
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            //pos.z += vel.z * dt * 10.0;
        }
    }
	
    // reincarnation
	if( age >= maxAge || reset )
    {
        vec3 origVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 origPos = texture2D(oPositions, texCoord.st).rgb;
        
        age = 0.0;
        
        if (reset) {
            age = 0.5 * decay * 100.0;
        }
        
        if(pos.x > 1.0 || pos.x < 0.0 || pos.y > 1.0 || pos.y < 0.0 || reset) {
            pos = origPos;
        }
//        else {
//            pos = origPos + vel;
//        }
        
        vel = origVel;
    }
    
    // bounce off walls
    if (pos.x > 1.0 || pos.x < 0.0)
    {
        vel = vec3(vel.x*(-0.85), vel.y*0.9, 0.0);
        age *= 1.1;
    }
    if (pos.y > 1.0 || pos.y < 0.0)
    {
        vel = vec3(vel.x*0.9, -vel.y*(-0.85), 0.0);
        age *= 1.1;
    }
    
    // no stragglers
//    if (age > 0.9 && vel.x < 0.0001 && vel.y < 0.0001) {
//        vel *= 2.0;
//    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, invMass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, age);
    //age information
	gl_FragData[2] = vec4(decay, maxAge, startPos.x, startPos.y);
}
