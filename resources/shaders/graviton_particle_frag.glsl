//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform vec3 attractorPos1;
uniform vec3 attractorPos2;
uniform vec3 attractorPos3;
uniform float eps;
uniform float dt;
varying vec4 texCoord;

void main(void)
{
    vec3 p0 = texture2D( positions, texCoord.st).rgb;
    vec3 v0 = texture2D( velocities, texCoord.st).rgb;
    float invmass = texture2D( positions, texCoord.st).a;
 
    // the simulation
    
    vec3 f1 = attractorPos1-p0; //force
    float f1Mag = length(f1); //force magnitude
    vec3 f2 = attractorPos2-p0; //force
    float f2Mag = length(f2); //force magnitude
    vec3 f3 = attractorPos3-p0; //force
    float f3Mag = length(f3); //force magnitude
    
    vec3 a1 = 0.05 * invmass * f1/(f1Mag*f1Mag + eps);
    vec3 a2 = 0.05 * invmass * f2/(f2Mag*f2Mag + eps);
    vec3 a3 = 0.05 * invmass * f3/(f3Mag*f3Mag + eps);
    
    vec3 v1 = v0 + dt * a1; //velocity update
    v1 += dt * a2;
    v1 += dt * a3;
    v1 = v1 - 0.02 * v1; //friction
    vec3 p1	= p0 + dt * v1; //(symplectic euler) position update
    
    //Render to positions texture
    gl_FragData[0] = vec4(p1, invmass);
    //Render to velocities texture
    gl_FragData[1] = vec4(v1, 0.5+length(v0)); //alpha component used for coloring
} 

