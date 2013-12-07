uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float     iBackgroundAlpha;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform vec4      iColor3;
uniform float     iDialation;
uniform float     iPatternAmp;
uniform float     iPatternFreq;
uniform float     iScale;

// based on https://www.shadertoy.com/view/lsfGRr
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}


float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    
    f = f*f*(3.0-2.0*f);
    
    float n = p.x + p.y*57.0;
    
    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

// fractional brownian motion
float fbm( vec2 p )
{
    float f = 0.0;
    
    // frequency = amp * noise
    f += 0.50000*noise( p ); p = m*p*2.02;
    f += 0.25000*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p ); p = m*p*2.04;
    f += 0.03125*noise( p );
    
    return f/0.984375;
}



float length2( vec2 p )
{
    float ax = abs(p.x);
    float ay = abs(p.y);
    
    return pow( pow(ax,4.0) + pow(ay,4.0), 1.0/4.0 );
}


void main(void)
{
    vec2 q = gl_FragCoord.xy / iResolution.xy;
    // centered, left to right
    vec2 p = -1.0 + 2.0 * q;
    p /= iScale;
    
    // r = dist from center
    // a = atan of y/x
    
    p.x *= iResolution.x/iResolution.y;
    
    float r = length( p );
    float a = atan( p.y, p.x );
    
    // dd = dialation
    float dd = 1.0 - iDialation;//0.2*sin(4.0*iGlobalTime);
    float ss = 1.0 + clamp(1.0-r,0.0,1.0)*dd;
    
    r *= ss;
    
    // coloration
    vec3 col = iColor1.xyz;
    // col1 = vec3( 0.0, 0.3, 0.4 );
    // col2 = vec3(0.2,0.5,0.4)
    // col3 = vec3(0.9,0.6,0.2)
    
    // f = noise for color
    float f = fbm( 5.0*p );
    col = mix( col, iColor2.xyz, f );
    
    col = mix( col, iColor3.xyz, 1.0-smoothstep(0.2,0.6,r) );
    
    // amp = 0.05
    // freq = 20.0
    a += iPatternAmp*fbm( iPatternFreq*p );
    
    // white strands
    // brightness range, lower f = more white
    // x = frequency, y = length ... adjust x to hide seam
    f = smoothstep( 0.3, 1.0, fbm( vec2(19.0*a,6.0*r) ) );
    col = mix( col, vec3(1.0,1.0,1.0), f );
    
    // dark strands
    f = smoothstep( 0.4, 0.9, fbm( vec2(15.0*a,10.0*r) ) );
    col *= 1.0-0.5*f;
    
    // iris
    col *= 1.0-0.25*smoothstep( 0.6,0.8,r );
    
    // light
    f = 1.0-smoothstep( 0.0, 0.6, length2( mat2(0.6,0.8,-0.8,0.6)*(p-vec2(0.3,0.5) )*vec2(1.0,2.0)) );
    
    col += vec3(1.0,0.9,0.9)*f*0.585;
    
    // dark tint
    col *= vec3(0.8+0.2*cos(r*a));
    
    // iris edge gradient
    f = 1.0-smoothstep( 0.2, 0.25, r );
    col = mix( col, vec3(0.0), f );
    
    f = smoothstep( 0.79, 0.82, r );
    col *= 0.5 + 0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.1);
    
    // background
    vec4 cola = mix( vec4(col,1.0), vec4(0.0,0.0,0.0,iBackgroundAlpha), f );

	gl_FragColor = cola;//vec4(col,iBackgroundAlpha);
}
