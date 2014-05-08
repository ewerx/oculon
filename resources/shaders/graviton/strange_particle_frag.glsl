uniform sampler2D positions;
uniform sampler2D velocities;
uniform float s;
uniform float b;
uniform float r;
uniform float dt;
uniform bool noiseSim;
varying vec4 texCoord;

void main(void)
{
    vec3 p0 = texture2D( positions, texCoord.st).rgb;
    vec3 v0 = texture2D( velocities, texCoord.st).rgb;
    float invmass = texture2D( positions, texCoord.st).a;
 
    vec3 p1;
    
    // WAT?
    p1.x = p0.x + s * (p0.y - p0.x);
    p1.y = p0.y + p0.x * (r - p0.z) - p0.y;
    p1.z = p0.z + p0.x * p0.y - b * p0.z;
    
    //Render to positions texture
    gl_FragData[0] = vec4(p1, invmass);
    //Render to velocities texture
    //gl_FragData[1] = vec4(v1, 0.5+length(v0)); //alpha component used for coloring
} 

