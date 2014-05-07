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

uniform bool reset;

varying vec4 texCoord;


void main(void)
{
    vec3 p0 = texture2D( positions, texCoord.st).rgb;
    vec3 v0 = texture2D( velocities, texCoord.st).rgb;
    float invmass = texture2D( positions, texCoord.st ).a;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float decay = texture2D( information, texCoord.st ).a;
 
    // the simulation
    
    vec3 f1 = attractorPos1-p0; //force
    float f1Mag = length(f1); //force magnitude
    vec3 f2 = attractorPos2-p0; //force
    float f2Mag = length(f2); //force magnitude
    //vec3 f3 = attractorPos3-p0; //force
    //float f3Mag = length(f3); //force magnitude
    
    vec3 a1 = gravity * invmass * f1/(f1Mag*f1Mag + eps);
    vec3 a2 = gravity * invmass * f2/(f2Mag*f2Mag + eps);
    vec3 a3 = vec3(0.0,0.0,0.0);//gravity * invmass * f3/(f3Mag*f3Mag + eps);
    
    vec3 v1 = v0 + dt * (a1 + a2 + a3); //velocity update
    v1 = v1 - damping * v1; //friction/damping
    vec3 p1	= p0 + dt * v1; //(symplectic euler) position update
    
    // contain within sphere
    float dist = length(p1);
    if (containerradius > 5.0 && dist > containerradius || dist > 1000.0) {
        vec3 norm = normalize(p1);
        p1 = norm * containerradius;
        v1 *= 0.9;
    }
    
    // reincarnation
	if( reset )
    {
        vec3 origVel = texture2D(oVelocities, texCoord.st).rgb;
        vec3 origPos = texture2D(oPositions, texCoord.st).rgb;
        p1 = origPos;
        v1 = origVel;
    }
    
    float age = 0.5+length(v0); // for coloring
    
    //Render to positions texture
    gl_FragData[0] = vec4(p1, invmass);
    //Render to velocities texture
    gl_FragData[1] = vec4(v1, 0.5+length(v0)); //alpha component used for coloring
    //age information
	gl_FragData[2] = vec4(startPos, decay);
}
