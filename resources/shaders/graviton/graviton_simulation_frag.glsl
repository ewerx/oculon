//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;
//uniform vec3 attractorPos1;
//uniform vec3 attractorPos2;
//uniform vec3 attractorPos3;
//uniform vec3 attractorPos4;
//uniform float attractorMass1;
//uniform float attractorMass2;
//uniform float attractorMass3;
//uniform float attractorMass4;
uniform float eps;
uniform float dt;
uniform float damping;
uniform float gravity;
uniform float gravity2;
uniform float containerradius;

uniform bool reset;
uniform bool startAnim;
uniform float formationStep;

uniform sampler2D nodePosTex;
uniform float nodeBufSize;

varying vec4 texCoord;

void reactToNodes( inout vec3 acc, vec3 particlePos, float invmass )
{
    float nInvFboDim = 1.0 / nodeBufSize;
    int nFboDim = int(nodeBufSize);
    
    for( int y=0; y<nFboDim; y++ ){
        for( int x=0; x<nFboDim; x++ ){
            vec2 tc				= vec2( float(x), float(y) ) * nInvFboDim + nInvFboDim * 0.5;
            vec3 nodePos		= texture2D( nodePosTex, tc ).rgb;
            
            vec3 dirToNode			= nodePos - particlePos;
            float distToNode		= length( dirToNode );
            float distToNodeSqrd	= distToNode * distToNode;
            
            acc = acc + ( gravity * invmass * ( dirToNode / (distToNodeSqrd + eps) ) );
            //float per = gravity * invmass * ( distToNodeSqrd + eps );
            //acc += normalize( dirToNode ) * per * 0.12 * dt;
        }
    }
}

void main()
{
    vec3 pos = texture2D( positions, texCoord.st ).rgb;
    float invmass = texture2D( positions, texCoord.st ).a;
    
    vec3 vel = texture2D( velocities, texCoord.st ).rgb;
    float age = texture2D( velocities, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
    
    if (startAnim) {
        vel = vec3(0.0);
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
//        // gravity simulation
//        vec3 f1 = attractorPos1 - pos; //force
//        float f1Mag = length(f1); //force magnitude
//        vec3 f2 = attractorPos2 - pos; //force
//        float f2Mag = length(f2); //force magnitude
//        vec3 f3 = attractorPos3 - pos; //force
//        float f3Mag = length(f3); //force magnitude
//        vec3 f4 = attractorPos4 - pos; //force
//        float f4Mag = length(f4); //force magnitude
//        
//        vec3 a1 = gravity * invmass * f1/(f1Mag*f1Mag + eps);
//        vec3 a2 = gravity2 * invmass * f2/(f2Mag*f2Mag + eps);
//        vec3 a3 = gravity * invmass * f3/(f3Mag*f3Mag + eps);
//        vec3 a4 = gravity2 * invmass * f4/(f4Mag*f4Mag + eps);
//        
//        vel = vel + dt * (a1 + a2 + a3 + a4); //velocity update
        
        vec3 acc = vec3(0.0);
        reactToNodes( acc, pos, invmass );
        vel = vel + dt * acc;
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
