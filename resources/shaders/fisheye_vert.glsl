void main (void)
{
    // Eye-coordinate position of vertex, needed in various calculations
    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    
    vec3 eye3 = vec3(ecPosition) * (1.0 / ecPosition.w);
    
    // change to polar coordinates
    float radius = 50.0;  // fisheye hemisphere radius
    vec3 p = eye3;
    vec3 pn = p;
    float d = length(p);
    pn = normalize(p);
    d = d / radius;
    float u = atan(pn.y,pn.x);
    float v = 2.0 * acos(-pn.z) / 3.141529265358979;
    
    // change polar to cartesian coordinates on circle (with depth)
    gl_Position.x = cos(u) * v;
    gl_Position.y = sin(u) * v;
    gl_Position.z = d;// * -1.0 * abs(p.z) / p.z;
    gl_Position.w = 1.0;
    
    vec3 normal = gl_NormalMatrix * gl_Normal;
    normal = normalize(normal);
    
    // pass on the texture coordinate
    gl_TexCoord[0] = gl_MultiTexCoord0;
}