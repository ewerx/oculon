//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform float dt;

uniform bool reset;
uniform bool startAnim;
uniform float formationStep;

uniform vec3 harmonic;

varying vec4 texCoord;

vec3 applyHarmonic(vec3 targetPos, vec3 velocity)
{
    float dist = length(targetPos);
    vec3 newPos;
    newPos.x = dist * cos(velocity.x) * sin(velocity.y);
    newPos.y = dist * sin(velocity.x) * sin(velocity.y);
    newPos.z = dist * cos(velocity.y);
    return newPos;
}

void main()
{
    vec3 pos = texture2D( positions, texCoord.st ).rgb;
    float invmass = texture2D( positions, texCoord.st ).a;
    
    vec3 vel = texture2D( velocities, texCoord.st ).rgb;
    float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
    vec3 oVel = texture2D(oVelocities, texCoord.st).rgb;
    vec3 oPos = texture2D(oPositions, texCoord.st).rgb;
    
    // animate to formation
    if (startAnim)
    {
        //vel = oVel; // spin
        //startPos = pos; // animate from current position
        
        
        // TODO: add a flatten sim that just flattens
        // then go from formation -> flat -> formation to get weird effects
        // or formation -> gravity -> flat -> formation ?
        // add more interesting gravity options
        // move nodes into a particle controller for more options?
        
        // flatten then animated position into information tex
        // after animation is done, animate actual position to pos stored in information
        
        
        // flatten
        vec3 newVel = harmonic * age;
        vec3 newPos = applyHarmonic(oPos, newVel);
        
        vel = newVel;
        startPos = pos;
    }
    
    if( reset )
    {
        // reincarnation
        pos = oPos;
        vel = oVel;
    }
    else
    {
        if (formationStep < 1.0)
        {
            vec3 newVel = harmonic * age;
            vec3 newPos = applyHarmonic(oPos, newVel);
            
            vel = newVel;
            
            pos = mix(startPos, newPos, formationStep);
        }
        else
        {
            vec3 newVel = vel;
            // vel.x = rho
            // vel.y = theta
            // vel.z = 1.0 for animate rho, 0.0 for animate theta
            // vel.a (age) = speed
            if (oVel.z > 0.5)
            {
                newVel.x = newVel.x + dt * age ;
            }
            else
            {
                newVel.y = newVel.y + dt * age;
            }
            
            vec3 newPos = applyHarmonic(oPos, newVel);
            
            pos = newPos;
            vel = newVel;
        }
    }
    
    //float age = 0.5+length(v0); // for coloring
    
    //Render to positions texture
    gl_FragData[0] = vec4(pos, invmass);
    //Render to velocities texture
    gl_FragData[1] = vec4(vel, age); //alpha component used for coloring
    //age information
	gl_FragData[2] = vec4(startPos, decay);
}