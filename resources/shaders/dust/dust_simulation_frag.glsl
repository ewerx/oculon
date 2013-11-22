#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform sampler2D noiseTex;
uniform float dt;
uniform float decayRate;

varying vec4 texCoord;

void main()
{
    
	vec3 pos = texture2D( positions, texCoord.st).rgb;
	float mass = texture2D( positions, texCoord.st).a;

	vec3 vel = texture2D( velocities, texCoord.st).rgb;
	float age = texture2D( velocities, texCoord.st).a;
    
	float decay = texture2D( information, texCoord.st).r;
	float maxAge = texture2D( information, texCoord.st).g;

    vec2 noise = texture2D( noiseTex, pos.xy).rg;
    
    age += dt * decay * decayRate;
    
	vel += vec3(noise.x,noise.y,0.0);
    
    pos.x += vel.x * dt * 10.0;
    pos.y += vel.y * dt * 10.0;
	
    // reincarnation
	if( age >= maxAge )
    {
        vec3 origVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 origPos = texture2D(oPositions, texCoord.st).rgb;
        
        age = decay * 0.001;
        
        if(pos.x > 1024.0 || pos.x < 0.0 || pos.y > 1024.0 || pos.y < 0.0 ) {
            pos = origPos;
        } else {
            pos = origPos + vel;
        }
        
        vel = origVel;
    }
    
    // bounce off walls
    if (pos.x > 1024.0 || pos.x < 0.0 || pos.y > 1024.0 || pos.y < 0.0)
    {
        vel *= -0.9;
        age *= 0.85;
    }
    
    // no stragglers
//    if (age > 0.9 && vel.x < 0.0001 && vel.y < 0.0001) {
//        vel *= 2.0;
//    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, mass);
    //velocity + decay
	gl_FragData[1] = vec4(vel, age);
    //age information
	//gl_FragData[2] = vec4(age, maxAge, 0.0, 1.0);
}
