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

varying vec4 texCoord;


void main()
{
    vec3 pos = texture2D( positions, texCoord.st ).rgb;
    float invmass = texture2D( positions, texCoord.st ).a;
    
    vec3 vel = texture2D( velocities, texCoord.st ).rgb;
    float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
    // animate to formation
    if (startAnim)
    {
        startPos = pos;
    }
    
    if( reset )
    {
        // reincarnation
        pos = texture2D(oPositions, texCoord.st).rgb;
        vel = texture2D(oVelocities, texCoord.st).rgb;
    }
    else
    {
        vec3 oVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 oPos = texture2D(oPositions, texCoord.st).rgb;
        // vel.x = rho
        // vel.y = theta
        // vel.z = 1.0 for animate rho, 0.0 for animate theta
        // vel.a (age) = speed
        if (oVel.z > 0.5)
        {
            vel.x = vel.x + dt * age;
        }
        else
        {
            vel.y = vel.y + dt * age;
        }
        
        float dist = length(oPos);
        vec3 newPos;
        
        newPos.x = dist * cos(vel.x) * sin(vel.y);
        newPos.y = dist * sin(vel.x) * sin(vel.y);
        newPos.z = dist * cos(vel.y) * sin(vel.x);
        
        if (formationStep < 1.0)
        {
            //vel = vec3(0.0,0.0,0.0);
            pos = mix(startPos, newPos, formationStep);
        }
        else
        {
            pos = newPos;
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
