//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
uniform vec3 attractorPos1;
uniform vec3 attractorPos2;
uniform vec3 attractorPos3;
uniform float eps;
uniform float dt;
uniform float damping;
uniform float gravity;
uniform float containerradius;
uniform float formationStep;

uniform bool reset;
uniform bool startAnim;

varying vec4 texCoord;


void main()
{
    vec3 pos = texture2D( positions, texCoord.st ).rgb;
    float invmass = texture2D( positions, texCoord.st ).a;
    
    vec3 vel = texture2D( velocities, texCoord.st ).rgb;
    float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
    if (startAnim)
    {
        startPos = pos;
    }
    
    if (formationStep < 1.0)
    {
        // animate to formation
        vel = vec3(0.0,0.0,0.0);
        vec3 targetPos = texture2D( oPositions, texCoord.st ).rgb;
        
        pos = mix(startPos,targetPos,formationStep);
    }
	else if( reset )
    {
        // reincarnation
        pos = texture2D(oPositions, texCoord.st).rgb;
        vel = texture2D(oVelocities, texCoord.st).rgb;
    }
    else
    {
        // gravity simulation
        vec3 f1 = attractorPos1 - pos; //force
        float f1Mag = length(f1); //force magnitude
        vec3 f2 = attractorPos2 - pos; //force
        float f2Mag = length(f2); //force magnitude
        //vec3 f3 = attractorPos3-p0; //force
        //float f3Mag = length(f3); //force magnitude
        
        vec3 a1 = gravity * invmass * f1/(f1Mag*f1Mag + eps);
        vec3 a2 = gravity * invmass * f2/(f2Mag*f2Mag + eps);
        vec3 a3 = vec3(0.0,0.0,0.0);//gravity * invmass * f3/(f3Mag*f3Mag + eps);
        
        vel = vel + dt * (a1 + a2 + a3); //velocity update
        vel = vel - damping * vel; //friction/damping
        pos	= pos + dt * vel; //(symplectic euler) position update
        
        // contain within sphere
        float dist = length(pos);
        if (containerradius > 5.0 && dist > containerradius || dist > 1000.0)
        {
            vec3 norm = normalize(pos);
            pos = norm * containerradius;
            vel *= 0.9;
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
