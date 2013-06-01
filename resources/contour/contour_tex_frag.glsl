uniform float theta;
uniform vec3  scale;
uniform vec3  colorScale;
uniform sampler1D tex;

varying vec2 uv;
const float twoPi = 3.1415926 * 2.0;

//
// Ashima Noise
//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//	 https://github.com/ashima/webgl-noise/tree/master/demo/common
//
// Adapted by Roger Sodre
//


/*
uniform float	u_time;
uniform vec3	u_scale;
uniform vec2	u_RenderSize;
*/

float lmap(float val, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
}

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise3d(vec3 v)
{
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);

    vec4 p = permute( permute( permute(
    i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
    + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
    + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.

    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)

    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)
    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;
    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));

    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);

    m = m * m;

    float n = 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );

    return lmap( n, -1.0, 1.0, 0.0, 1.0 );
}

float snoise3d(float x, float y, float z)
{
    return snoise3d( vec3(x,y,z) );
}
void main( void )
{
//	vec2 offset		= vec2( cos( theta * twoPi ), sin( theta * twoPi ) ) * 0.25 + vec2( 0.5, 0.5 );
//	float dist		= distance( uv, offset );
//	vec4 the_color	= vec4( dist, 0.0, 0.0, 1.0 );
    vec2 coord = uv;//gl_TexCoord[0].st;
    vec3 v = vec3( coord, theta ) * scale;
    float n = snoise3d( v );
    
    //gl_FragColor = vec4( n, 0.0, 0.0, 1.0 );
    vec4 color			= vec4( 0.0, 0.0, 0.0, 1.0 );
    //color               = texture1D( tex, n );
    float levels = 64.0;
    float band = floor(levels * n);
    float value = band / levels;
//    if (((levels * n) - band) < 0.05) {
//        color.x = 0.0;
//        color.y = 0.0;
//        color.z = 0.0;
//    } else {
    {
        color.x = colorScale.x * value;
        color.y = colorScale.y * value;
        color.z = colorScale.z * value;
    }
    gl_FragColor = color;
}

/*
//

// main()

//

void main( void )

{

// pixel coord for GL_TEXTURE_2D

vec2 coord = gl_TexCoord[0].st;

// pixel coord GL_TEXTURE_RECTANGLE_ARB

//vec2 coord = gl_FragCoord.st / u_RenderSize;

vec3 v = vec3( coord, u_time ) * u_scale;

float n = snoise3d( v );

vec4 the_color = vec4( n, n, n, 1.0 );

the_color *= gl_Color;

gl_FragColor = the_color;

}
*/